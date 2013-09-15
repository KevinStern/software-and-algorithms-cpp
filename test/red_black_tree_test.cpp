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

#include <algorithm>
#include <set>
#include <vector>

#include "red_black_tree.h"

template <typename Collection, typename Tree>
static void equals_helper(const Collection& master, const Tree& tree) {
  ASSERT_EQ(master.size(), tree.size());
  for (auto iter = master.begin(); iter != master.end(); ++iter) {
    ASSERT_TRUE(tree.contains(*iter));
  }
}

TEST(RedBlackTreeTestContains) {
  RedBlackTree<int, Node<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 0; j < 100; j++) {
    ASSERT_FALSE(tree.contains(j));
    tree.insert(j);
    ASSERT_TRUE(tree.contains(j));
  }
}

TEST(RedBlackTreeTestDelete) {
  std::vector<int> master;
  RedBlackTree<int, Node<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 0; j < 100; j++) {
    tree.insert(j);
    master.push_back(j);
  }
  while (!master.empty()) {
    int val = master[master.size() > 1];
    master.erase(find(master.begin(), master.end(), val));
    tree.remove(val);
    equals_helper(master, tree);
  }
}

TEST(RedBlackTreeTestInsert) {
  std::set<int> master;
  RedBlackTree<int, Node<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 99; j >= 0; j--) {
    tree.insert(j);
    master.insert(j);
  }
  equals_helper(master, tree);
}

TEST(RedBlackTreeTestPredecessor) {
  RedBlackTree<int, Node<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 0; j < 100; j++) {
    tree.insert(j);
  }
  for (int j = 1; j < 100; j++) {
    ASSERT_EQ((j - 1), tree.predecessor(tree.node(j))->value());
  }
  ASSERT_NULL(tree.predecessor(tree.node(0)));
}

TEST(RedBlackTreeTestSuccessor) {
  RedBlackTree<int, Node<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 0; j < 100; j++) {
    tree.insert(j);
  }
  for (int j = 0; j < 99; j++) {
    ASSERT_EQ((j + 1), tree.successor(tree.node(j))->value());
  }
  ASSERT_NULL(tree.successor(tree.node(99)));
}

TEST(RedBlackTreeTestLinkedPredecessor) {
  RedBlackTree<int, LinkedNode<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 0; j < 100; j++) {
    tree.insert(j);
  }
  for (int j = 1; j < 100; j++) {
    ASSERT_EQ((j - 1), tree.predecessor(tree.node(j))->value());
  }
  ASSERT_NULL(tree.predecessor(tree.node(0)));
}

TEST(RedBlackTreeTestLinkedSuccessor) {
  RedBlackTree<int, LinkedNode<int>> tree([](const int& o1, const int& o2)->int{return o1 - o2;});
  for (int j = 0; j < 100; j++) {
    tree.insert(j);
  }
  for (int j = 0; j < 99; j++) {
    ASSERT_EQ((j + 1), tree.successor(tree.node(j))->value());
  }
  ASSERT_NULL(tree.successor(tree.node(99)));
}
