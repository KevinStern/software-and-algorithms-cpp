/* Copyright (c) 2012 Kevin L. Stern
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef HUNGARIAN_H_
#define HUNGARIAN_H_

#include <algorithm>
#include <limits>
#include <boost/multi_array.hpp>

/**
 * An implementation of the Hungarian algorithm for solving the assignment
 * problem. An instance of the assignment problem consists of a number of
 * workers along with a number of jobs and a cost matrix which gives the cost of
 * assigning the i'th worker to the j'th job at position (i, j). The goal is to
 * find an assignment of workers to jobs so that no job is assigned more than
 * one worker and so that no worker is assigned to more than one job in such a
 * manner so as to minimize the total cost of completing the jobs.
 * <p>
 *
 * An assignment for a cost matrix that has more workers than jobs will
 * necessarily include unassigned workers, indicated by an assignment value of
 * UNASSIGNED; in no other circumstance will there be unassigned workers. Similarly, an
 * assignment for a cost matrix that has more jobs than workers will necessarily
 * include unassigned jobs; in no other circumstance will there be unassigned
 * jobs. For completeness, an assignment for a square cost matrix will give
 * exactly one unique worker to each job.
 * <p>
 *
 * This version of the Hungarian algorithm runs in time O(n^3), where n is the
 * maximum among the number of workers and the number of jobs.
 *
 * @author Kevin L. Stern
 */
class Hungarian {
public:
	static const uint32_t UNASSIGNED;

	Hungarian(const boost::multi_array<double, 2> &costMatrix) :
			rows(costMatrix.size()), cols(costMatrix[0].size()), dim(
					std::max(rows, cols)), cost_matrix(
					boost::extents[dim][dim]), label_by_worker(
					boost::extents[dim]), label_by_job(boost::extents[dim]), min_slack_by_job(
					boost::extents[dim]), min_slack_worker_by_job(
					boost::extents[dim]), match_job_by_worker(
					boost::extents[dim]), match_worker_by_job(
					boost::extents[dim]), parent_worker_by_committed_job(
					boost::extents[dim]), committed_workers(boost::extents[dim]) {
		for (uint32_t w = 0; w < dim; ++w) {
			if (w < rows) {
				uint32_t j = 0;
				for (; j < cols; ++j) {
					this->cost_matrix[w][j] = costMatrix[w][j];
				}
				for (; j < dim; ++j) {
					this->cost_matrix[w][j] = 0;
				}
			} else {
				for (uint32_t j = 0; j < dim; ++j) {
					this->cost_matrix[w][j] = 0;
				}
			}
		}
		for (uint32_t w = 0; w < match_job_by_worker.size(); ++w) {
			match_job_by_worker[w] = UNASSIGNED;
		}
		for (uint32_t j = 0; j < match_worker_by_job.size(); ++j) {
			match_worker_by_job[j] = UNASSIGNED;
		}
	}

	/**
	 * Execute the algorithm.
	 *
	 * @return the minimum cost matching of workers to jobs based upon the
	 *         provided cost matrix. A matching value of UNASSIGNED indicates that the
	 *         corresponding worker is unassigned.
	 */
	void execute(uint32_t result[]) {
		/*
		 * Heuristics to improve performance: Reduce rows and columns by their
		 * smallest element, compute an initial non-zero dual feasible solution
		 * and create a greedy matching from workers to jobs of the cost matrix.
		 */
		reduce();
		computeInitialFeasibleSolution();
		greedyMatch();

		uint32_t w = fetchUnmatchedWorker();
		while (w < dim) {
			initializePhase(w);
			executePhase();
			w = fetchUnmatchedWorker();
		}
		memcpy(result, match_job_by_worker.data(), sizeof(uint32_t) * rows);
		for (w = 0; w < rows; ++w) {
			if (result[w] >= cols) {
				result[w] = UNASSIGNED;
			}
		}
	}

protected:
	/**
	 * Compute an initial feasible solution by assigning zero labels to the
	 * workers and by assigning to each job a label equal to the minimum cost
	 * among its incident edges.
	 */
	void computeInitialFeasibleSolution() {
		for (uint32_t j = 0; j < dim; ++j) {
			label_by_job[j] = POSITIVE_INFINITY;
		}
		for (uint32_t w = 0; w < dim; ++w) {
			for (uint32_t j = 0; j < dim; ++j) {
				if (cost_matrix[w][j] < label_by_job[j]) {
					label_by_job[j] = cost_matrix[w][j];
				}
			}
		}
	}

	/**
	 * Execute a single phase of the algorithm. A phase of the Hungarian
	 * algorithm consists of building a set of committed workers and a set of
	 * committed jobs from a root unmatched worker by following alternating
	 * unmatched/matched zero-slack edges. If an unmatched job is encountered,
	 * then an augmenting path has been found and the matching is grown. If the
	 * connected zero-slack edges have been exhausted, the labels of committed
	 * workers are increased by the minimum slack among committed workers and
	 * non-committed jobs to create more zero-slack edges (the labels of
	 * committed jobs are simultaneously decreased by the same amount in order
	 * to maintain a feasible labeling).
	 * <p>
	 *
	 * The runtime of a single phase of the algorithm is O(n^2), where n is the
	 * dimension of the internal square cost matrix, since each edge is visited
	 * at most once and since increasing the labeling is accomplished in time
	 * O(n) by maintaining the minimum slack values among non-committed jobs.
	 * When a phase completes, the matching will have increased in size.
	 */
	void executePhase() {
		while (true) {
			uint32_t min_slack_worker = UNASSIGNED, min_slack_job = UNASSIGNED;
			double min_slack_value = POSITIVE_INFINITY;
			for (uint32_t j = 0; j < dim; ++j) {
				if (parent_worker_by_committed_job[j] == UNASSIGNED) {
					if (min_slack_by_job[j] < min_slack_value) {
						min_slack_value = min_slack_by_job[j];
						min_slack_worker = min_slack_worker_by_job[j];
						min_slack_job = j;
					}
				}
			}
			if (min_slack_value > 0) {
				updateLabeling(min_slack_value);
			}
			parent_worker_by_committed_job[min_slack_job] = min_slack_worker;
			if (match_worker_by_job[min_slack_job] == UNASSIGNED) {
				/*
				 * An augmenting path has been found.
				 */
				uint32_t committed_job = min_slack_job;
				uint32_t parent_worker =
						parent_worker_by_committed_job[committed_job];
				while (true) {
					uint32_t temp = match_job_by_worker[parent_worker];
					match(parent_worker, committed_job);
					committed_job = temp;
					if (committed_job == UNASSIGNED) {
						break;
					}
					parent_worker =
							parent_worker_by_committed_job[committed_job];
				}
				return;
			} else {
				/*
				 * Update slack values since we increased the size of the
				 * committed workers set.
				 */
				uint32_t worker = match_worker_by_job[min_slack_job];
				committed_workers[worker] = true;
				for (uint32_t j = 0; j < dim; ++j) {
					if (parent_worker_by_committed_job[j] == UNASSIGNED) {
						double slack = cost_matrix[worker][j]
								- label_by_worker[worker] - label_by_job[j];
						if (min_slack_by_job[j] > slack) {
							min_slack_by_job[j] = slack;
							min_slack_worker_by_job[j] = worker;
						}
					}
				}
			}
		}
	}

	/**
	 *
	 * @return the first unmatched worker or {@link #dim} if none.
	 */
	uint32_t fetchUnmatchedWorker() {
		uint32_t w;
		for (w = 0; w < dim; ++w) {
			if (match_job_by_worker[w] == UNASSIGNED) {
				break;
			}
		}
		return w;
	}

	/**
	 * Find a valid matching by greedily selecting among zero-cost matchings.
	 * This is a heuristic to jump-start the augmentation algorithm.
	 */
	void greedyMatch() {
		for (uint32_t w = 0; w < dim; ++w) {
			for (uint32_t j = 0; j < dim; ++j) {
				if (match_job_by_worker[w] == UNASSIGNED
						&& match_worker_by_job[j] == UNASSIGNED
						&& cost_matrix[w][j] - label_by_worker[w]
								- label_by_job[j] == 0) {
					match(w, j);
				}
			}
		}
	}

	/**
	 * Initialize the next phase of the algorithm by clearing the committed
	 * workers and jobs sets and by initializing the slack arrays to the values
	 * corresponding to the specified root worker.
	 *
	 * @w
	 *            the worker at which to root the next phase.
	 */
	void initializePhase(uint32_t w) {
		memset(committed_workers.data(), false,
				committed_workers.size() * sizeof(bool));
		for (uint32_t j = 0; j < dim; ++j) {
			parent_worker_by_committed_job[j] = UNASSIGNED;
		}
		committed_workers[w] = true;
		for (uint32_t j = 0; j < dim; ++j) {
			min_slack_by_job[j] = cost_matrix[w][j] - label_by_worker[w]
					- label_by_job[j];
			min_slack_worker_by_job[j] = w;
		}
	}

	/**
	 * Helper method to record a matching between worker w and job j.
	 */
	void match(uint32_t w, uint32_t j) {
		match_job_by_worker[w] = j;
		match_worker_by_job[j] = w;
	}

	/**
	 * Reduce the cost matrix by subtracting the smallest element of each row
	 * from all elements of the row as well as the smallest element of each
	 * column from all elements of the column. Note that an optimal assignment
	 * for a reduced cost matrix is optimal for the original cost matrix.
	 */
	void reduce() {
		for (uint32_t w = 0; w < dim; ++w) {
			double min = POSITIVE_INFINITY;
			for (uint32_t j = 0; j < dim; ++j) {
				if (cost_matrix[w][j] < min) {
					min = cost_matrix[w][j];
				}
			}
			for (uint32_t j = 0; j < dim; ++j) {
				cost_matrix[w][j] -= min;
			}
		}
		{
			boost::multi_array<double, 1> min(boost::extents[dim]);
			for (uint32_t j = 0; j < dim; ++j) {
				min[j] = POSITIVE_INFINITY;
			}
			for (uint32_t w = 0; w < dim; ++w) {
				for (uint32_t j = 0; j < dim; ++j) {
					if (cost_matrix[w][j] < min[j]) {
						min[j] = cost_matrix[w][j];
					}
				}
			}
			for (uint32_t w = 0; w < dim; ++w) {
				for (uint32_t j = 0; j < dim; ++j) {
					cost_matrix[w][j] -= min[j];
				}
			}
		}
	}

	/**
	 * Update labels with the specified slack by adding the slack value for
	 * committed workers and by subtracting the slack value for committed jobs.
	 * In addition, update the minimum slack values appropriately.
	 */
	void updateLabeling(double slack) {
		for (uint32_t w = 0; w < dim; ++w) {
			if (committed_workers[w]) {
				label_by_worker[w] += slack;
			}
		}
		for (uint32_t j = 0; j < dim; ++j) {
			if (parent_worker_by_committed_job[j] != UNASSIGNED) {
				label_by_job[j] -= slack;
			} else {
				min_slack_by_job[j] -= slack;
			}
		}
	}
private:
	static const double POSITIVE_INFINITY;

	uint32_t rows, cols, dim;
	boost::multi_array<double, 2> cost_matrix;
	boost::multi_array<double, 1> label_by_worker, label_by_job,
			min_slack_by_job;
	boost::multi_array<uint32_t, 1> min_slack_worker_by_job,
			match_job_by_worker, match_worker_by_job,
			parent_worker_by_committed_job;
	boost::multi_array<bool, 1> committed_workers;
};

const double Hungarian::POSITIVE_INFINITY = std::numeric_limits<double>::max();
const uint32_t Hungarian::UNASSIGNED = std::numeric_limits<uint32_t>::max();

#endif /* HUNGARIAN_H_ */
