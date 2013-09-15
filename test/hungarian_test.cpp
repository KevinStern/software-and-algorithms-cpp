/* Copyright (c) 2013 Kevin L. Stern
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
#include "test.h"

#include <unordered_set>

#include "hungarian.h"

static double compute_cost(MultiArray<double, 2>& matrix, uint32_t* match) {
  double result = 0;
  std::unordered_set<size_t> visited;
  for (int i = 0; i < matrix.size(); ++i) {
    if (match[i] == -1) {
      continue;
    }
    ASSERT_TRUE(visited.insert(match[i]).second);
    result += matrix[i][match[i]];
  }
  return result;
}

TEST(HungarianTest1) {
  MultiArray<double, 2> matrix({{4., 1.5, 4.},
                                {4., 4.5, 6.},
                                {3., 2.25, 3.}});
  Hungarian b(matrix);
  uint32_t match[3];
  b.execute(match);
  uint32_t expected[3] {1, 0, 2};
  ASSERT_ARRAY_EQ(expected, match, 3);
  ASSERT_EQ(8.5, compute_cost(matrix, match), 0.0000001);
}

TEST(HungarianTest2) {
  MultiArray<double, 2> matrix({{ 1.0, 1.0, 0.8 },
                                { 0.9, 0.8, 0.1 },
                                { 0.9, 0.7, 0.4 }});
  Hungarian b(matrix);
  uint32_t match[3];
  b.execute(match);
  uint32_t expected[3] {0, 2, 1};
  ASSERT_ARRAY_EQ(expected, match, 3);
  ASSERT_EQ(1.8, compute_cost(matrix, match), 0.0000001);
}

TEST(HungarianTest3) {
  MultiArray<double, 2> matrix({{ 6.0, 0.0, 7.0, 5.0 },
                                { 2.0, 6.0, 2.0, 6.0 },
                                { 2.0, 7.0, 2.0, 1.0 },
                                { 9.0, 4.0, 7.0, 1.0 }});
  Hungarian b(matrix);
  uint32_t match[4];
  b.execute(match);
  uint32_t expected[4] {1, 0, 2, 3};
  ASSERT_ARRAY_EQ(expected, match, 4);
  ASSERT_EQ(5, compute_cost(matrix, match), 0.0000001);
}

TEST(HungarianTestUnassignedJob) {
  MultiArray<double, 2> matrix({{ 6.0, 0.0, 7.0, 5.0, 2.0 },
                                { 2.0, 6.0, 2.0, 6.0, 7.0 },
                                { 2.0, 7.0, 2.0, 1.0, 1.0 },
                                { 9.0, 4.0, 7.0, 1.0, 0.0 }});
  Hungarian b(matrix);
  uint32_t match[4];
  b.execute(match);
  uint32_t expected[4] {1, 0, 3, 4};
  ASSERT_ARRAY_EQ(expected, match, 4);
  ASSERT_EQ(3, compute_cost(matrix, match), 0.0000001);
}

TEST(HungarianTestUnassignedWorker) {
  MultiArray<double, 2> matrix({{ 6.0, 0.0, 7.0, 5.0 },
                                { 2.0, 6.0, 2.0, 6.0 },
                                { 2.0, 7.0, 2.0, 1.0 },
                                { 9.0, 4.0, 7.0, 1.0 },
                                { 0.0, 0.0, 0.0, 0.0 }});
  Hungarian b(matrix);
  uint32_t match[5];
  b.execute(match);
  uint32_t expected[5] {1, Hungarian::UNASSIGNED, 2, 3, 0};
  ASSERT_ARRAY_EQ(expected, match, 5);
  ASSERT_EQ(3, compute_cost(matrix, match), 0.0000001);
}
