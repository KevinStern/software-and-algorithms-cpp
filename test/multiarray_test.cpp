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

#include "multiarray.h"

TEST(MultiArrayBasic) {
  MultiArray<int, 1> one = {1, 2, 3, 4};
  ASSERT_EQ(4, one.size());
  for (int i = 0; i < one.size(); ++i) {
    ASSERT_EQ(i + 1, one[i]);
  }

  MultiArray<int, 2> two = {{1, 2}, {3, 4}};
  ASSERT_EQ(2, two.size());
  for (int i = 0; i < two.size(); ++i) {
    ASSERT_EQ(2, two[i].size());
    for (int j = 0; j < two[i].size(); ++j) {
      ASSERT_EQ(j + 1 + (2 * i), two[i][j]);
    }
  }
}

TEST(MultiArrayOutOfBounds) {
  MultiArray<int, 1> one = {1, 2, 3, 4};
  try {
    one[4];
    ASSERT_TRUE(false);
  } catch (...) {
  }

  MultiArray<int, 2> two = {{1, 2}, {3, 4}};
  try {
    two[2][0];
    ASSERT_TRUE(false);
  } catch (...) {
  }
  try {
    two[0][2];
    ASSERT_TRUE(false);
  } catch (...) {
  }
}
