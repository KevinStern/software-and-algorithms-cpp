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

#include <string>

#include "knuth_morris_pratt.h"

TEST(KnuthMorrisPrattTest1) {
  std::string needle = "needle";
  std::string haystack = "It's like searching for a needle in a haystack.";
  ASSERT_EQ(haystack.find(needle), KnuthMorrisPratt(needle).execute(haystack));
}

TEST(KnuthMorrisPrattTest2) {
  std::string needle = "01012";
  std::string haystack = "010101012";
  ASSERT_EQ(haystack.find(needle), KnuthMorrisPratt(needle).execute(haystack));
}

TEST(KnuthMorrisPrattTest3) {
  std::string needle = "0101";
  std::string haystack = "0102020101";
  ASSERT_EQ(haystack.find(needle), KnuthMorrisPratt(needle).execute(haystack));
}

TEST(KnuthMorrisPrattTest4) {
  std::string needle = "aaaaaaa";
  std::string haystack = "aaaaaab";
  ASSERT_EQ(KnuthMorrisPratt::NOT_FOUND, KnuthMorrisPratt(needle).execute(haystack));
}

TEST(KnuthMorrisPrattTest5) {
  std::string needle = "aaaaaaa";
  std::string haystack = "aaaaaaa";
  ASSERT_EQ(KnuthMorrisPratt::NOT_FOUND, KnuthMorrisPratt(needle).execute(haystack, 1));
}

TEST(KnuthMorrisPrattTest6) {
  std::string needle = "aa";
  std::string haystack = "aaaaaaa";
  ASSERT_EQ(haystack.find(needle, 1), KnuthMorrisPratt(needle).execute(haystack, 1));
}
