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

#ifndef DAMERAU_LEVENSHTEIN_H_
#define DAMERAU_LEVENSHTEIN_H_

#include <tr1/unordered_map>
#include <stdint.h>
#include <string>
#include <algorithm>
#include <limits>

#include "multiarray.h"

/**
 * The Damerau-Levenshtein Algorithm is an extension to the Levenshtein
 * Algorithm which solves the edit distance problem between a source string and
 * a target string with the following operations:
 *
 * <ul>
 * <li>Character Insertion</li>
 * <li>Character Deletion</li>
 * <li>Character Replacement</li>
 * <li>Adjacent Character Swap</li>
 * </ul>
 *
 * Note that the adjacent character swap operation is an edit that may be
 * applied when two adjacent characters in the source string match two adjacent
 * characters in the target string, but in reverse order, rather than a general
 * allowance for adjacent character swaps.
 * <p>
 *
 * This implementation allows the client to specify the costs of the various
 * edit operations with the restriction that the cost of two swap operations
 * must not be less than the cost of a delete operation followed by an insert
 * operation. This restriction is required to preclude two swaps involving the
 * same character being required for optimality which, in turn, enables a fast
 * dynamic programming solution.
 * <p>
 *
 * The running time of the Damerau-Levenshtein algorithm is O(n*m) where n is
 * the length of the source string and m is the length of the target string.
 * This implementation consumes O(n*m) space.
 *
 * @author Kevin L. Stern
 */
class DamerauLevenshtein {
public:
	/**
	 * Constructor.
	 *
	 * @param deleteCost
	 *            the cost of deleting a character.
	 * @param insertCost
	 *            the cost of inserting a character.
	 * @param replaceCost
	 *            the cost of replacing a character.
	 * @param swapCost
	 *            the cost of swapping two adjacent characters.
	 */
	DamerauLevenshtein(int32_t insertCost, int32_t deleteCost,
			int32_t replaceCost, int32_t swapCost) :
			insert_cost(insertCost), delete_cost(deleteCost), replace_cost(
					replaceCost), swap_cost(swapCost) {
		if (2 * swapCost < insertCost + deleteCost) {
			throw "Invalid Argument";
		}
	}

	/**
	 * Compute the Damerau-Levenshtein distance between the specified source
	 * string and the specified target string.
	 */
	int32_t execute(const std::string &source,
			const std::string &target) const {
		if (source.size() == 0) {
			return target.size() * insert_cost;
		}
		if (target.size() == 0) {
			return source.size() * delete_cost;
		}
		std::tr1::unordered_map<char, uint32_t> source_index_by_char;
		MultiArray<int32_t, 2> table(source.size(), target.size());
		if (source[0] == target[0]) {
			table[0][0] = 0;
		} else {
			table[0][0] = std::min(this->insert_cost + this->delete_cost,
					this->replace_cost);
		}
		source_index_by_char[source[0]] = 0;
		for (uint32_t i = 1; i < source.size(); ++i) {
			int32_t del = table[i - 1][0] + this->delete_cost;
			int32_t ins = (i + 1) * this->delete_cost + this->insert_cost;
			int32_t repl = i * this->delete_cost
					+ (source[i] == target[0] ? 0 : this->replace_cost);
			table[i][0] = std::min(del, std::min(ins, repl));
		}
		for (uint32_t j = 1; j < target.size(); ++j) {
			int32_t del = table[0][j - 1] + this->insert_cost;
			int32_t ins = (j + 1) * this->insert_cost + this->delete_cost;
			int32_t repl = j * this->insert_cost
					+ (source[0] == target[j] ? 0 : this->replace_cost);
			table[0][j] = std::min(del, std::min(ins, repl));
		}
		const uint32_t uint32_MAX = std::numeric_limits<uint32_t>::max();
		const uint32_t int32_MAX = std::numeric_limits<int32_t>::max();
		for (uint32_t i = 1; i < source.size(); ++i) {
			uint32_t max_source_letter_match_index =
					source[i] == target[0] ? 0 : uint32_MAX;
			for (uint32_t j = 1; j < target.size(); ++j) {
				uint32_t j_swap = max_source_letter_match_index;
				int32_t del = table[i - 1][j] + this->delete_cost;
				int32_t ins = table[i][j - 1] + this->insert_cost;
				int32_t repl = table[i - 1][j - 1];
				if (source[i] != target[j]) {
					repl += this->replace_cost;
				} else {
					max_source_letter_match_index = j;
				}
				int32_t swap;
				std::tr1::unordered_map<char, uint32_t>::const_iterator find =
						source_index_by_char.find(target[j]);
				if (find != source_index_by_char.end()
						&& j_swap != uint32_MAX) {
					uint32_t i_swap = find->second;
					uint32_t pre_swap_cost;
					if (i_swap == 0 && j_swap == 0) {
						pre_swap_cost = 0;
					} else {
						pre_swap_cost = table[i_swap == 0 ? 0 : i_swap - 1][
								j_swap == 0 ? 0 : j_swap - 1];
					}
					swap = pre_swap_cost + (i - i_swap - 1) * delete_cost
							+ (j - j_swap - 1) * insert_cost + swap_cost;
				} else {
					swap = int32_MAX;
				}

				table[i][j] = std::min(swap,
						std::min(del, std::min(ins, repl)));
			}
			source_index_by_char[source[i]] = i;
		}

		return table[source.size() - 1][target.size() - 1];
	}
private:
	int32_t insert_cost, delete_cost, replace_cost, swap_cost;
};

#endif /* DAMERAU_LEVENSHTEIN_H_ */
