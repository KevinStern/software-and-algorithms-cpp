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

#include "damerau_levenshtein.h"

TEST(DamerauLevenshteinTest) {
  ASSERT_EQ(7, DamerauLevenshtein(1, 1, 1, 1).execute("NawKtYu", ""));
  ASSERT_EQ(7, DamerauLevenshtein(1, 1, 1, 1).execute("", "NawKtYu"));
  ASSERT_EQ(0, DamerauLevenshtein(1, 1, 1, 1).execute("NawKtYu", "NawKtYu"));
  ASSERT_EQ(6, DamerauLevenshtein(1, 1, 1, 1).execute("NawKtYu", "tKNwYua"));
  ASSERT_EQ(1, DamerauLevenshtein(1, 1, 1, 1).execute("Jdc", "dJc"));
  ASSERT_EQ(5, DamerauLevenshtein(1, 1, 1, 1).execute("sUzSOwx", "zsSxUwO"));
  ASSERT_EQ(7, DamerauLevenshtein(1, 1, 1, 1).execute("eOqoHAta", "tAeaqHoO"));
  ASSERT_EQ(1, DamerauLevenshtein(1, 1, 1, 1).execute("glSbo", "lgSbo"));
  ASSERT_EQ(4, DamerauLevenshtein(1, 1, 1, 1).execute("NJtQKcJE", "cJEtQKJN"));
  ASSERT_EQ(5, DamerauLevenshtein(1, 1, 1, 1).execute("GitIEVs", "EGItVis"));
  ASSERT_EQ(4, DamerauLevenshtein(1, 1, 1, 1).execute("MiWK", "WKiM"));
}

TEST(DamerauLevenshteinCosts) {
  /*
   * Test replace cost.
   */
  ASSERT_EQ(1, DamerauLevenshtein(100, 100, 1, 100).execute("a", "b"));
  /*
   * Test swap cost.
   */
  ASSERT_EQ(200, DamerauLevenshtein(100, 100, 100, 200).execute("ab", "ba"));
  /*
   * Test delete cost.
   */
  ASSERT_EQ(1, DamerauLevenshtein(100, 1, 100, 100).execute("aa", "a"));
  /*
   * Test insert cost.
   */
  ASSERT_EQ(1, DamerauLevenshtein(1, 100, 100, 100).execute("a", "aa"));
}

TEST(DamerauLevenshteinInvalidCosts) {
  try {
    DamerauLevenshtein(1, 1, 1, 0);
    ASSERT_TRUE(false);
  } catch (...) {
  }
}
