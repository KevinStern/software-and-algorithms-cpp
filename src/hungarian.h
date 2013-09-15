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
#pragma once

#include <algorithm>
#include <limits>

#include "multiarray.h"

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
  static const uint32_t UNASSIGNED = std::numeric_limits<uint32_t>::max();

  Hungarian(const MultiArray<double, 2>& cost_matrix) :
      rows_(cost_matrix.size()), cols_(cost_matrix[0].size()), dim_(std::max(rows_, cols_)),
      cost_matrix_(dim_, dim_), label_by_worker_(dim_), label_by_job_(dim_),
      min_slack_by_job_(dim_), min_slack_worker_by_job_(dim_), match_job_by_worker_(dim_),
      match_worker_by_job_(dim_), parent_worker_by_committed_job_(dim_), committed_workers_(dim_) {
    for (uint32_t w = 0; w < dim_; ++w) {
      if (w < rows_) {
        uint32_t j = 0;
        for (; j < cols_; ++j) {
          cost_matrix_[w][j] = cost_matrix[w][j];
        }
        for (; j < dim_; ++j) {
          cost_matrix_[w][j] = 0;
        }
      } else {
        for (uint32_t j = 0; j < dim_; ++j) {
          cost_matrix_[w][j] = 0;
        }
      }
    }
    for (uint32_t w = 0; w < match_job_by_worker_.size(); ++w) {
      match_job_by_worker_[w] = UNASSIGNED;
    }
    for (uint32_t j = 0; j < match_worker_by_job_.size(); ++j) {
      match_worker_by_job_[j] = UNASSIGNED;
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
    compute_initial_feasible_solution();
    greedy_match();

    uint32_t w = fetch_unmatched_worker();
    while (w < dim_) {
      initialize_phase(w);
      execute_phase();
      w = fetch_unmatched_worker();
    }
    memcpy(result, match_job_by_worker_.data(), sizeof(uint32_t) * rows_);
    for (w = 0; w < rows_; ++w) {
      if (result[w] >= cols_) {
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
  void compute_initial_feasible_solution() {
    for (uint32_t j = 0; j < dim_; ++j) {
      label_by_job_[j] = POSITIVE_INFINITY;
    }
    for (uint32_t w = 0; w < dim_; ++w) {
      for (uint32_t j = 0; j < dim_; ++j) {
        if (cost_matrix_[w][j] < label_by_job_[j]) {
          label_by_job_[j] = cost_matrix_[w][j];
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
  void execute_phase() {
    while (true) {
      uint32_t min_slack_worker = UNASSIGNED, min_slack_job = UNASSIGNED;
      double min_slack_value = POSITIVE_INFINITY;
      for (uint32_t j = 0; j < dim_; ++j) {
        if (parent_worker_by_committed_job_[j] == UNASSIGNED) {
          if (min_slack_by_job_[j] < min_slack_value) {
            min_slack_value = min_slack_by_job_[j];
            min_slack_worker = min_slack_worker_by_job_[j];
            min_slack_job = j;
          }
        }
      }
      if (min_slack_value > 0) {
        update_labeling(min_slack_value);
      }
      parent_worker_by_committed_job_[min_slack_job] = min_slack_worker;
      if (match_worker_by_job_[min_slack_job] == UNASSIGNED) {
        /*
         * An augmenting path has been found.
         */
        uint32_t committed_job = min_slack_job;
        uint32_t parent_worker =
          parent_worker_by_committed_job_[committed_job];
        while (true) {
          uint32_t temp = match_job_by_worker_[parent_worker];
          match(parent_worker, committed_job);
          committed_job = temp;
          if (committed_job == UNASSIGNED) {
            break;
          }
          parent_worker =
            parent_worker_by_committed_job_[committed_job];
        }
        return;
      } else {
        /*
         * Update slack values since we increased the size of the
         * committed workers set.
         */
        uint32_t worker = match_worker_by_job_[min_slack_job];
        committed_workers_[worker] = true;
        for (uint32_t j = 0; j < dim_; ++j) {
          if (parent_worker_by_committed_job_[j] == UNASSIGNED) {
            double slack = cost_matrix_[worker][j] - label_by_worker_[worker] - label_by_job_[j];
            if (min_slack_by_job_[j] > slack) {
              min_slack_by_job_[j] = slack;
              min_slack_worker_by_job_[j] = worker;
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
  uint32_t fetch_unmatched_worker() {
    uint32_t w;
    for (w = 0; w < dim_; ++w) {
      if (match_job_by_worker_[w] == UNASSIGNED) {
        break;
      }
    }
    return w;
  }

  /**
   * Find a valid matching by greedily selecting among zero-cost matchings.
   * This is a heuristic to jump-start the augmentation algorithm.
   */
  void greedy_match() {
    for (uint32_t w = 0; w < dim_; ++w) {
      for (uint32_t j = 0; j < dim_; ++j) {
        if (match_job_by_worker_[w] == UNASSIGNED
            && match_worker_by_job_[j] == UNASSIGNED
            && cost_matrix_[w][j] - label_by_worker_[w] - label_by_job_[j] == 0) {
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
   * @w the worker at which to root the next phase.
   */
  void initialize_phase(uint32_t w) {
    memset(committed_workers_.data(), false, committed_workers_.size() * sizeof(bool));
    for (uint32_t j = 0; j < dim_; ++j) {
      parent_worker_by_committed_job_[j] = UNASSIGNED;
    }
    committed_workers_[w] = true;
    for (uint32_t j = 0; j < dim_; ++j) {
      min_slack_by_job_[j] = cost_matrix_[w][j] - label_by_worker_[w] - label_by_job_[j];
      min_slack_worker_by_job_[j] = w;
    }
  }

  /**
   * Helper method to record a matching between worker w and job j.
   */
  void match(uint32_t w, uint32_t j) {
    match_job_by_worker_[w] = j;
    match_worker_by_job_[j] = w;
  }

  /**
   * Reduce the cost matrix by subtracting the smallest element of each row
   * from all elements of the row as well as the smallest element of each
   * column from all elements of the column. Note that an optimal assignment
   * for a reduced cost matrix is optimal for the original cost matrix.
   */
  void reduce() {
    for (uint32_t w = 0; w < dim_; ++w) {
      double min = POSITIVE_INFINITY;
      for (uint32_t j = 0; j < dim_; ++j) {
        if (cost_matrix_[w][j] < min) {
          min = cost_matrix_[w][j];
        }
      }
      for (uint32_t j = 0; j < dim_; ++j) {
        cost_matrix_[w][j] -= min;
      }
    }
    {
      MultiArray<double, 1> min(dim_);
      for (uint32_t j = 0; j < dim_; ++j) {
        min[j] = POSITIVE_INFINITY;
      }
      for (uint32_t w = 0; w < dim_; ++w) {
        for (uint32_t j = 0; j < dim_; ++j) {
          if (cost_matrix_[w][j] < min[j]) {
            min[j] = cost_matrix_[w][j];
          }
        }
      }
      for (uint32_t w = 0; w < dim_; ++w) {
        for (uint32_t j = 0; j < dim_; ++j) {
          cost_matrix_[w][j] -= min[j];
        }
      }
    }
  }

  /**
   * Update labels with the specified slack by adding the slack value for
   * committed workers and by subtracting the slack value for committed jobs.
   * In addition, update the minimum slack values appropriately.
   */
  void update_labeling(double slack) {
    for (uint32_t w = 0; w < dim_; ++w) {
      if (committed_workers_[w]) {
        label_by_worker_[w] += slack;
      }
    }
    for (uint32_t j = 0; j < dim_; ++j) {
      if (parent_worker_by_committed_job_[j] != UNASSIGNED) {
        label_by_job_[j] -= slack;
      } else {
        min_slack_by_job_[j] -= slack;
      }
    }
  }

private:
  static constexpr double POSITIVE_INFINITY = std::numeric_limits<double>::max();

  uint32_t rows_, cols_, dim_;
  MultiArray<double, 2> cost_matrix_;
  MultiArray<double, 1> label_by_worker_, label_by_job_, min_slack_by_job_;
  MultiArray<uint32_t, 1> min_slack_worker_by_job_, match_job_by_worker_,
    match_worker_by_job_, parent_worker_by_committed_job_;
  MultiArray<bool, 1> committed_workers_;
};
