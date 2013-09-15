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

#include <cstdint>

#include <queue>

/**
 * red_black_tree.h
 *
 * A <em>red-black tree</em> is a binary search tree guaranteeing that no path
 * from root to leaf is more than twice as long as any other such path. This
 * property provides an assurance that the height of a red-black tree is
 * logarithmic in the number of nodes in the tree.
 * <p>
 * This implementation is based upon Cormen, Leiserson, Rivest, Stein's
 * Introduction to Algorithms book.
 *
 * @see Introduction to Algorithms Cormen, Leiserson, Rivest, and Stein.
 *      Introduction to Algorithms. 2nd ed. Cambridge, MA: MIT Press, 2001.
 *      ISBN: 0262032937.
 */
enum NodeColor {
  RED, BLACK
};

template<class T>
class Node;

template<class T>
class LinkedNode;

template<class T, class NodeType>
class RedBlackTree {
public:
  RedBlackTree(int (*compare)(const T&, const T&)) : compare_(compare), root_(nullptr), size_(0) {}

  ~RedBlackTree() {
    std::queue<NodeType*> q;
    if (root_ != nullptr) {
      q.push(root_);
    }
    while (q.size() > 0) {
      NodeType* node = q.front();
      q.pop();
      if (node->left() != nullptr) {
        q.push(node->left());
      }
      if (node->right() != nullptr) {
        q.push(node->right());
      }
      delete node;
    }
  }

  /**
   * Insert the specified value into this tree.
   *
   * @param value
   *            the value to insert.
   * @return true if the value was inserted to this tree, false otherwise.
   */
  bool insert(const T& value) {
    NodeType* node = nullptr;
    NodeType* parent = root_;
    while (parent != nullptr) {
      int delta = compare_(parent->value(), value);
      if (delta < 0) {
        if (parent->right() == nullptr) {
          node = new NodeType(value);
          parent->set_right(node);
          node->set_parent(parent);
          parent = nullptr;
        } else {
          parent = parent->right();
        }
      } else if (delta > 0) {
        if (parent->left() == nullptr) {
          node = new NodeType(value);
          parent->set_left(node);
          node->set_parent(parent);
          parent = nullptr;
        } else {
          parent = parent->left();
        }
      } else {
        return false;
      }
    }
    if (node == nullptr) {
      node = new NodeType(value);
      root_ = node;
    }

    set_color(node, RED);
    fix_after_insertion(node);
    ++size_;

    post_insert(node);

    return true;
  }

  /**
   * Remove the specified value from this tree.
   *
   * @param value
   *            the value to remove.
   * @return true if the value was removed from this tree, false otherwise.
   */
  bool remove(const T& value) {
    NodeType* node = this->node(value);
    if (node == nullptr)
      return false;
    NodeType* swap;
    if (!(node->left() == nullptr|| node->right() == nullptr)) {
      NodeType* successor = this->successor(node);
      exchange_values(node, successor);
      node = successor;
    }
    if (node->left() != nullptr) {
      swap = node->left();
    } else {
      swap = node->right();
    }
    if (swap != nullptr)
      swap->set_parent(node->parent());
    if (node->parent() == nullptr)
      root_ = swap;
    else if (node == node->parent()->left())
      node->parent()->set_left(swap);
    else
      node->parent()->set_right(swap);
    if (node->color() == BLACK) {
      if (root_ != nullptr)
        fix_after_removal(swap == nullptr ? node : swap);
    }

    --size_;

    post_delete(node);

    bool result = node != nullptr;
    delete node;
    return result;
  }

  uint32_t size() const {
    return size_;
  }

  NodeType* first_node() {
    return get_first_node_impl();
  }

  const NodeType* first_node() const {
    return get_first_node_impl();
  }

  NodeType* last_node() {
    return get_last_node_impl();
  }

  const NodeType* last_node() const {
    return get_last_node_impl();
  }

  /**
   * Get the node that stores the specified value.
   *
   * @param value
   *            the query value.
   * @return the node that stores the specified value, null if none.
   */
  NodeType* node(const T& value) {
    return get_node_impl(value);
  }

  /**
   * Get the node that stores the specified value.
   *
   * @param value
   *            the query value.
   * @return the node that stores the specified value, null if none.
   */
  const NodeType* node(const T& value) const {
    return get_node_impl(value);
  }

  NodeType* root() {
    return root_;
  }

  const NodeType* root() const {
    return root_;
  }

  /**
   * Get the predecessor of the specified node. The predecessor of a node n is
   * the node with the largest value in the tree smaller than the value stored
   * at n.
   *
   * @see CLRS
   */
  NodeType* predecessor(NodeType* node) {
    return predecessor_impl(node);
  }

  /**
   * Get the predecessor of the specified node. The predecessor of a node n is
   * the node with the largest value in the tree smaller than the value stored
   * at n.
   *
   * @see CLRS
   */
  const NodeType* predecessor(const NodeType* node) const {
    return predecessor_impl(node);
  }

  /**
   * Get the successor of the specified node. The successor of a node n is the
   * node with the smallest value in the tree larger than the value stored at n.
   *
   * @see CLRS
   */
  NodeType* successor(NodeType* node) {
    return successor_impl(node);
  }

  /**
   * Get the successor of the specified node. The successor of a node n is the
   * node with the smallest value in the tree larger than the value stored at n.
   *
   * @see CLRS
   */
  const NodeType* successor(const NodeType* node) const {
    return successor_impl(node);
  }

  /**
   * Test whether or not the specified value is an element of this tree.
   *
   * @param value
   *            the query value.
   * @return true if the specified value is an element of this tree, false
   *         otherwise.
   */
  bool contains(const T& value) const {
    return node(value) != nullptr;
  }

protected:
  /**
   * Perform a right rotate operation on the specified node.
   *
   * @param node
   *            the node on which a right rotate operation is to be performed.
   *
   * @see CLRS Introduction to Algorithms
   */
  void right_rotate(NodeType* node) {
    NodeType* temp = node->left();
    node->set_left(temp->right());
    if (temp->right() != nullptr)
      temp->right()->set_parent(node);
    temp->set_parent(node->parent());
    if (node->parent() == nullptr)
      root_ = temp;
    else if (node == node->parent()->right())
      node->parent()->set_right(temp);
    else
      node->parent()->set_left(temp);
    temp->set_right(node);
    node->set_parent(temp);
  }

  /**
   * Perform a left rotate operation on the specified node.
   *
   * @param node
   *            the node on which the left rotate operation will be performed.
   *
   * @see CLRS Introduction to Algorithms
   */
  void left_rotate(NodeType* node) {
    NodeType* temp = node->right();
    node->set_right(temp->left());
    if (temp->left() != nullptr)
      temp->left()->set_parent(node);

    temp->set_parent(node->parent());
    if (node->parent() == nullptr)
      root_ = temp;
    else if (node == node->parent()->left())
      node->parent()->set_left(temp);
    else
      node->parent()->set_right(temp);

    temp->set_left(node);
    node->set_parent(temp);
  }

  /**
   * Re-balance the tree after an insert operation.
   *
   * @param node
   *            the inserted node.
   *
   * @see CLRS Introduction to Algorithms
   */
  void fix_after_insertion(NodeType* node) {
    while (color(node->parent()) == RED) {
      if (node->parent() == node->parent()->parent()->left()) {
        NodeType* temp = node->parent()->parent()->right();
        if (color(temp) == RED) {
          set_color(node->parent(), BLACK);
          set_color(temp, BLACK);
          set_color(node->parent()->parent(), RED);
          node = node->parent()->parent();
        } else {
          if (node == node->parent()->right()) {
            node = node->parent();
            left_rotate(node);
          }
          set_color(node->parent(), BLACK);
          set_color(node->parent()->parent(), RED);
          right_rotate(node->parent()->parent());
        }
      } else {
        NodeType* temp = node->parent()->parent()->left();
        if (color(temp) == RED) {
          set_color(node->parent(), BLACK);
          set_color(temp, BLACK);
          set_color(node->parent()->parent(), RED);
          node = node->parent()->parent();
        } else {
          if (node == node->parent()->left()) {
            node = node->parent();
            right_rotate(node);
          }
          set_color(node->parent(), BLACK);
          set_color(node->parent()->parent(), RED);
          left_rotate(node->parent()->parent());
        }
      }
    }
    set_color(root_, BLACK);
  }

  /**
   * Re-balance the tree after a remove operation.
   *
   * @param node
   *            the removed node or the swap node.
   *
   * @see CLRS Introduction to Algorithms
   */
  void fix_after_removal(NodeType* node) {
    while (node != root_ && color(node) == BLACK) {
      if (node == node->parent()->left()
          || (node->parent()->right() != nullptr && node != node->parent()->right())) {
        NodeType* temp = node->parent()->right();
        if (color(temp) == RED) {
          set_color(temp, BLACK);
          set_color(node->parent(), RED);
          left_rotate(node->parent());
          temp = node->parent()->right();
        }
        if (color(temp->left()) == BLACK && color(temp->right()) == BLACK) {
          set_color(temp, RED);
          node = node->parent();
        } else {
          if (color(temp->right()) == BLACK) {
            set_color(temp->left(), BLACK);
            set_color(temp, RED);
            right_rotate(temp);
            temp = node->parent()->right();
          }
          set_color(temp, color(node->parent()));
          set_color(node->parent(), BLACK);
          set_color(temp->right(), BLACK);
          left_rotate(node->parent());
          node = root_;
        }
      } else {
        NodeType* temp = node->parent()->left();
        if (color(temp) == RED) {
          set_color(temp, BLACK);
          set_color(node->parent(), RED);
          right_rotate(node->parent());
          temp = node->parent()->left();
        }
        if (color(temp->right()) == BLACK && color(temp->left()) == BLACK) {
          set_color(temp, RED);
          node = node->parent();
        } else {
          if (color(temp->left()) == BLACK) {
            set_color(temp->right(), BLACK);
            set_color(temp, RED);
            left_rotate(temp);
            temp = node->parent()->left();
          }
          set_color(temp, color(node->parent()));
          set_color(node->parent(), BLACK);
          set_color(temp->left(), BLACK);
          right_rotate(node->parent());
          node = root_;
        }
      }
    }
    set_color(node, BLACK);
  }

  /**
   * Called by {@link #remove(Node)} when the node to be removed is a leaf. In
   * this case, the node's value is exchanged with its successor as per the
   * typical binary tree node removal operation. This method allows a subclass
   * to influence value exchange behavior (e.g. if additional node information
   * needs to be exchanged).
   *
   * @param node
   *            the node whose value is to be removed.
   * @param successor
   *            the node to actually be removed.
   */
  void exchange_values(NodeType* n, NodeType* successor) {
    const T tempValue = successor->value();
    successor->set_value(n->value());
    n->set_value(tempValue);
    post_exchange_values(n, successor);
  }

private:
  int (*compare_)(const T&, const T&);
  NodeType* root_;
  uint32_t size_;

  inline void set_color(NodeType* node, NodeColor color) {
    if (node != nullptr) {
      node->set_color(color);
    }
  }

  /**
   * Convenience method implementing the concept of a null-node leaf being
   * black.
   *
   * @param node
   *            the node whose color is to be determined, null is interpreted
   *            as a null leaf and is assigned the color black.
   * @return the color of the specified node.
   */
  inline NodeColor color(NodeType* node) {
    return node == nullptr ? BLACK : node->color();
  }

  inline NodeType* get_first_node_impl() const {
    NodeType* result = root;
    if (result != nullptr) {
      while (result->left() != nullptr) {
        result = result->left();
      }
    }
    return result;
  }

  inline NodeType* get_last_node_impl() const {
    NodeType* result = root;
    if (result != nullptr) {
      while (result->right() != nullptr) {
        result = result->right();
      }
    }
    return result;
  }

  inline NodeType* get_node_impl(const T& value) const {
    NodeType* node = root_;
    while (node != nullptr) {
      int delta = compare_(node->value(), value);
      if (delta < 0) {
        node = node->right();
      } else if (delta > 0) {
        node = node->left();
      } else {
        break;
      }
    }
    return node;
  }

  inline NodeType* predecessor_internal(const NodeType* node) const {
    if (node == nullptr) {
      return nullptr;
    }
    NodeType* result = const_cast<NodeType*>(node);
    if (result->left() != nullptr) {
      result = result->left();
      while (result->right() != nullptr)
        result = result->right();
      return result;
    }
    NodeType* temp = result->parent();
    while (temp != nullptr && result == temp->left()) {
      result = temp;
      temp = temp->parent();
    }
    return temp;
  }

  inline NodeType* predecessor_impl(const Node<T>* node) const {
    return predecessor_internal(node);
  }

  inline NodeType* predecessor_impl(const LinkedNode<T>* node) const {
    return const_cast<NodeType*>(node)->predecessor();
  }

  inline NodeType* successor_internal(const NodeType* node) const {
    if (node == nullptr) {
      return nullptr;
    }
    NodeType* result = const_cast<NodeType*>(node);
    if (result->right() != nullptr) {
      result = result->right();
      while (result->left() != nullptr)
        result = result->left();
      return result;
    }
    NodeType* temp = result->parent();
    while (temp != nullptr && node == temp->right()) {
      node = temp;
      temp = temp->parent();
    }
    return temp;
  }

  inline NodeType* successor_impl(const Node<T>* node) const {
    return successor_internal(node);
  }

  inline NodeType* successor_impl(const LinkedNode<T>* node) const {
    return const_cast<NodeType*>(node)->successor();
  }

  inline void post_insert(Node<T>* node) {
    // no op
  }

  inline void post_insert(LinkedNode<T>* linkedNode) {
    NodeType* pred = predecessor_internal(linkedNode);
    linkedNode->set_predecessor(pred);
    if (pred != nullptr) {
      pred->set_successor(linkedNode);
    }
    NodeType* succ = successor_internal(linkedNode);
    linkedNode->set_successor(succ);
    if (succ != nullptr) {
      succ->set_predecessor(linkedNode);
    }
  }

  inline void post_delete(Node<T>* node) {
    // no op
  }

  inline void post_delete(LinkedNode<T>* linkedNode) {
    if (linkedNode->predecessor() != nullptr) {
      linkedNode->predecessor()->set_successor(linkedNode->successor());
    }
    if (linkedNode->successor() != nullptr) {
      linkedNode->successor()->set_predecessor(linkedNode->predecessor());
    }
  }

  inline void post_exchange_values(Node<T>* n, Node<T>* successor) {
    // no op
  }

  inline void post_exchange_values(LinkedNode<T>* linkedNode,
      LinkedNode<T>* linkedSuccessor) {
    linkedNode->set_successor(linkedSuccessor->successor());
    if (linkedNode->successor() != nullptr) {
      linkedNode->successor()->set_predecessor(linkedNode);
    }
    linkedSuccessor->set_predecessor(nullptr);
    linkedSuccessor->set_successor(nullptr);
  }
};

template<class T>
class Node {
public:
  Node(T value) : color_(BLACK), left_(nullptr), right_(nullptr), parent_(nullptr), value_(value) {}

  NodeColor color() const {
    return color_;
  }

  Node* left() {
    return left_;
  }

  const Node* left() const {
    return left_;
  }

  Node* right() {
    return right_;
  }

  const Node* right() const {
    return right_;
  }

  Node* parent() {
    return parent_;
  }

  const Node* parent() const {
    return parent_;
  }

  const T& value() const {
    return value_;
  }

  bool is_leaf() const {
    return left_ == nullptr && right_ == nullptr;
  }

private:
  NodeColor color_;
  Node* left_;
  Node* right_;
  Node* parent_;
  T value_;

  void set_left(Node* node) {
    left_ = node;
  }

  void set_right(Node* node) {
    right_ = node;
  }

  void set_parent(Node* node) {
    parent_ = node;
  }

  void set_value(const T& value) {
    value_ = value;
  }

  void set_color(NodeColor color) {
    color_ = color;
  }

  friend class RedBlackTree<T, Node>;
};

template<class T>
class LinkedNode {
public:
  LinkedNode(T value) : color_(BLACK), left_(nullptr), right_(nullptr), parent_(nullptr), value_(value),
      successor_(nullptr), predecessor_(nullptr) {}

  NodeColor color() const {
    return color_;
  }

  LinkedNode* left() {
    return left_;
  }

  const LinkedNode* left() const {
    return left_;
  }

  LinkedNode* right() {
    return right_;
  }

  const LinkedNode* right() const {
    return right_;
  }

  LinkedNode* parent() {
    return parent_;
  }

  const LinkedNode* parent() const {
    return parent_;
  }

  const T& value() const {
    return value_;
  }

  bool is_leaf() const {
    return left_ == nullptr && right_ == nullptr;
  }

  LinkedNode* successor() {
    return successor_;
  }

  const LinkedNode* successor() const {
    return successor_;
  }

  LinkedNode* predecessor() {
    return predecessor_;
  }

  const LinkedNode* predecessor() const {
    return predecessor_;
  }

private:
  NodeColor color_;
  LinkedNode* left_;
  LinkedNode* right_;
  LinkedNode* parent_;
  T value_;
  LinkedNode* successor_;
  LinkedNode* predecessor_;

  void set_left(LinkedNode* node) {
    left_ = node;
  }

  void set_right(LinkedNode* node) {
    right_ = node;
  }

  void set_parent(LinkedNode* node) {
    parent_ = node;
  }

  void set_value(const T& value) {
    value_ = value;
  }

  void set_color(NodeColor color) {
    color_ = color;
  }

  void set_successor(LinkedNode* node) {
    successor_ = node;
  }

  void set_predecessor(LinkedNode* node) {
    predecessor_ = node;
  }

  friend class RedBlackTree<T, LinkedNode>;
};
