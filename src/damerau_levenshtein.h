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
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>

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
   * @param delete_cost
   *            the cost of deleting a character.
   * @param insert_cost
   *            the cost of inserting a character.
   * @param replace_cost
   *            the cost of replacing a character.
   * @param swap_cost
   *            the cost of swapping two adjacent characters.
   */
  DamerauLevenshtein(int32_t insert_cost, int32_t delete_cost, int32_t replace_cost, int32_t swap_cost)
      : insert_cost_(insert_cost), delete_cost_(delete_cost), replace_cost_(replace_cost),
        swap_cost_(swap_cost) {
      if (2 * swap_cost < insert_cost + delete_cost) {
        throw std::runtime_error("Invalid Argument");
      }
    }

  /**
   * Compute the Damerau-Levenshtein distance between the specified source
   * string and the specified target string.
   */
  int32_t execute(const std::string& source, const std::string& target) const {
    if (source.size() == 0) {
      return target.size() * insert_cost_;
    }
    if (target.size() == 0) {
      return source.size() * delete_cost_;
    }
    std::unordered_map<char, uint32_t> source_index_by_char;
    MultiArray<int32_t, 2> table(source.size(), target.size());
    if (source[0] == target[0]) {
      table[0][0] = 0;
    } else {
      table[0][0] = std::min(insert_cost_ + delete_cost_, replace_cost_);
    }
    source_index_by_char[source[0]] = 0;
    for (uint32_t i = 1; i < source.size(); ++i) {
      int32_t del = table[i - 1][0] + delete_cost_;
      int32_t ins = (i + 1) * delete_cost_ + insert_cost_;
      int32_t repl = i * delete_cost_ + (source[i] == target[0] ? 0 : replace_cost_);
      table[i][0] = std::min(del, std::min(ins, repl));
    }
    for (uint32_t j = 1; j < target.size(); ++j) {
      int32_t del = (j + 1) * insert_cost_ + delete_cost_;
      int32_t ins = table[0][j - 1] + insert_cost_;
      int32_t repl = j * insert_cost_ + (source[0] == target[j] ? 0 : replace_cost_);
      table[0][j] = std::min(del, std::min(ins, repl));
    }
    const uint32_t uint32_MAX = std::numeric_limits<uint32_t>::max();
    const uint32_t int32_MAX = std::numeric_limits<int32_t>::max();
    for (uint32_t i = 1; i < source.size(); ++i) {
      uint32_t max_source_letter_match_index =
        source[i] == target[0] ? 0 : uint32_MAX;
      for (uint32_t j = 1; j < target.size(); ++j) {
        uint32_t j_swap = max_source_letter_match_index;
        int32_t del = table[i - 1][j] + delete_cost_;
        int32_t ins = table[i][j - 1] + insert_cost_;
        int32_t repl = table[i - 1][j - 1];
        if (source[i] != target[j]) {
          repl += replace_cost_;
        } else {
          max_source_letter_match_index = j;
        }
        int32_t swap;
        std::unordered_map<char, uint32_t>::const_iterator find =
          source_index_by_char.find(target[j]);
        if (find != source_index_by_char.end() && j_swap != uint32_MAX) {
          uint32_t i_swap = find->second;
          uint32_t pre_swap_cost_;
          if (i_swap == 0 && j_swap == 0) {
            pre_swap_cost_ = 0;
          } else {
            pre_swap_cost_ = table[i_swap == 0 ? 0 : i_swap - 1][
              j_swap == 0 ? 0 : j_swap - 1];
          }
          swap = pre_swap_cost_ + (i - i_swap - 1) * delete_cost_ + (j - j_swap - 1) * insert_cost_
                 + swap_cost_;
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
  int32_t insert_cost_, delete_cost_, replace_cost_, swap_cost_;
};
