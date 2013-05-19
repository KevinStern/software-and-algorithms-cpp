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

#ifndef RED_BLACK_TREE_H_
#define RED_BLACK_TREE_H_

#include <cstdint>

#ifndef NULL
#define	NULL	0
#endif

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
	RedBlackTree(int (*compare)(const T&, const T&)) :
			compare(compare), root(NULL), size(0) {
	}

	~RedBlackTree() {
		std::queue<NodeType*> q;
		if (root != NULL) {
			q.push(root);
		}
		while (q.size() > 0) {
			NodeType *node = q.front();
			q.pop();
			if (node->getLeft() != NULL) {
				q.push(node->getLeft());
			}
			if (node->getRight() != NULL) {
				q.push(node->getRight());
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
	bool insert(const T &value) {
		NodeType *node = NULL;
		NodeType *parent = root;
		while (parent != NULL) {
			int delta = compare(parent->getValue(), value);
			if (delta < 0) {
				if (parent->getRight() == NULL) {
					node = new NodeType(value);
					parent->setRight(node);
					node->setParent(parent);
					parent = NULL;
				} else {
					parent = parent->getRight();
				}
			} else if (delta > 0) {
				if (parent->getLeft() == NULL) {
					node = new NodeType(value);
					parent->setLeft(node);
					node->setParent(parent);
					parent = NULL;
				} else {
					parent = parent->getLeft();
				}
			} else {
				return false;
			}
		}
		if (node == NULL) {
			node = new NodeType(value);
			root = node;
		}

		setColor(node, RED);
		fixAfterInsertion(node);
		++size;

		postInsert(node);

		return true;
	}

	/**
	 * Remove the specified value from this tree.
	 *
	 * @param value
	 *            the value to remove.
	 * @return true if the value was removed from this tree, false otherwise.
	 */
	bool remove(const T &value) {
		NodeType *node = getNode(value);
		if (node == NULL)
			return false;
		NodeType *swap;
		if (!(node->getLeft() == NULL || node->getRight() == NULL)) {
			NodeType *successor = getSuccessor(node);
			exchangeValues(node, successor);
			node = successor;
		}
		if (node->getLeft() != NULL) {
			swap = node->getLeft();
		} else {
			swap = node->getRight();
		}
		if (swap != NULL)
			swap->setParent(node->getParent());
		if (node->getParent() == NULL)
			root = swap;
		else if (node == node->getParent()->getLeft())
			node->getParent()->setLeft(swap);
		else
			node->getParent()->setRight(swap);
		if (node->getColor() == BLACK) {
			if (root != NULL)
				fixAfterRemoval(swap == NULL ? node : swap);
		}

		--size;

		postDelete(node);

		bool result = node != NULL;
		delete node;
		return result;
	}

	uint32_t getSize() {
		return size;
	}

	NodeType* getFirstNode() {
		return getFirstNodeImpl();
	}

	const NodeType* getFirstNode() const {
		return getFirstNodeImpl();
	}

	NodeType* getLastNode() {
		return getLastNodeImpl();
	}

	const NodeType* getLastNode() const {
		return getLastNodeImpl();
	}

	/**
	 * Get the node that stores the specified value.
	 *
	 * @param value
	 *            the query value.
	 * @return the node that stores the specified value, null if none.
	 */
	NodeType* getNode(const T &value) {
		return getNodeImpl(value);
	}

	/**
	 * Get the node that stores the specified value.
	 *
	 * @param value
	 *            the query value.
	 * @return the node that stores the specified value, null if none.
	 */
	const NodeType* getNode(const T &value) const {
		return getNodeImpl(value);
	}

	NodeType* getRoot() {
		return root;
	}

	const NodeType* getRoot() const {
		return root;
	}

	/**
	 * Get the predecessor of the specified node. The predecessor of a node n is
	 * the node with the largest value in the tree smaller than the value stored
	 * at n.
	 *
	 * @see CLRS
	 */
	NodeType* getPredecessor(NodeType *node) {
		return getPredecessorImpl(node);
	}

	/**
	 * Get the predecessor of the specified node. The predecessor of a node n is
	 * the node with the largest value in the tree smaller than the value stored
	 * at n.
	 *
	 * @see CLRS
	 */
	const NodeType* getPredecessor(const NodeType *node) const {
		return getPredecessorImpl(node);
	}

	/**
	 * Get the successor of the specified node. The successor of a node n is the
	 * node with the smallest value in the tree larger than the value stored at n.
	 *
	 * @see CLRS
	 */
	NodeType* getSuccessor(NodeType *node) {
		return getSuccessorImpl(node);
	}

	/**
	 * Get the successor of the specified node. The successor of a node n is the
	 * node with the smallest value in the tree larger than the value stored at n.
	 *
	 * @see CLRS
	 */
	const NodeType* getSuccessor(const NodeType *node) const {
		return getSuccessorImpl(node);
	}

	/**
	 * Test whether or not the specified value is an element of this tree.
	 *
	 * @param value
	 *            the query value.
	 * @return true if the specified value is an element of this tree, false
	 *         otherwise.
	 */
	bool contains(const T &value) const {
		return getNode(value) != NULL;
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
	void rightRotate(NodeType *node) {
		NodeType *temp = node->getLeft();
		node->setLeft(temp->getRight());
		if (temp->getRight() != NULL)
			temp->getRight()->setParent(node);
		temp->setParent(node->getParent());
		if (node->getParent() == NULL)
			root = temp;
		else if (node == node->getParent()->getRight())
			node->getParent()->setRight(temp);
		else
			node->getParent()->setLeft(temp);
		temp->setRight(node);
		node->setParent(temp);
	}

	/**
	 * Perform a left rotate operation on the specified node.
	 *
	 * @param node
	 *            the node on which the left rotate operation will be performed.
	 *
	 * @see CLRS Introduction to Algorithms
	 */
	void leftRotate(NodeType *node) {
		NodeType * temp = node->getRight();
		node->setRight(temp->getLeft());
		if (temp->getLeft() != NULL)
			temp->getLeft()->setParent(node);

		temp->setParent(node->getParent());
		if (node->getParent() == NULL)
			root = temp;
		else if (node == node->getParent()->getLeft())
			node->getParent()->setLeft(temp);
		else
			node->getParent()->setRight(temp);

		temp->setLeft(node);
		node->setParent(temp);
	}

	/**
	 * Re-balance the tree after an insert operation.
	 *
	 * @param node
	 *            the inserted node.
	 *
	 * @see CLRS Introduction to Algorithms
	 */
	void fixAfterInsertion(NodeType *node) {
		while (getColor(node->getParent()) == RED) {
			if (node->getParent()
					== node->getParent()->getParent()->getLeft()) {
				NodeType *temp = node->getParent()->getParent()->getRight();
				if (getColor(temp) == RED) {
					setColor(node->getParent(), BLACK);
					setColor(temp, BLACK);
					setColor(node->getParent()->getParent(), RED);
					node = node->getParent()->getParent();
				} else {
					if (node == node->getParent()->getRight()) {
						node = node->getParent();
						leftRotate(node);
					}
					setColor(node->getParent(), BLACK);
					setColor(node->getParent()->getParent(), RED);
					rightRotate(node->getParent()->getParent());
				}
			} else {
				NodeType *temp = node->getParent()->getParent()->getLeft();
				if (getColor(temp) == RED) {
					setColor(node->getParent(), BLACK);
					setColor(temp, BLACK);
					setColor(node->getParent()->getParent(), RED);
					node = node->getParent()->getParent();
				} else {
					if (node == node->getParent()->getLeft()) {
						node = node->getParent();
						rightRotate(node);
					}
					setColor(node->getParent(), BLACK);
					setColor(node->getParent()->getParent(), RED);
					leftRotate(node->getParent()->getParent());
				}
			}
		}
		setColor(root, BLACK);
	}

	/**
	 * Re-balance the tree after a remove operation.
	 *
	 * @param node
	 *            the removed node or the swap node.
	 *
	 * @see CLRS Introduction to Algorithms
	 */
	void fixAfterRemoval(NodeType *node) {
		while (node != root && getColor(node) == BLACK) {
			if (node == node->getParent()->getLeft()
					|| (node->getParent()->getRight() != NULL
							&& node != node->getParent()->getRight())) {
				NodeType *temp = node->getParent()->getRight();
				if (getColor(temp) == RED) {
					setColor(temp, BLACK);
					setColor(node->getParent(), RED);
					leftRotate(node->getParent());
					temp = node->getParent()->getRight();
				}
				if (getColor(temp->getLeft()) == BLACK
						&& getColor(temp->getRight()) == BLACK) {
					setColor(temp, RED);
					node = node->getParent();
				} else {
					if (getColor(temp->getRight()) == BLACK) {
						setColor(temp->getLeft(), BLACK);
						setColor(temp, RED);
						rightRotate(temp);
						temp = node->getParent()->getRight();
					}
					setColor(temp, getColor(node->getParent()));
					setColor(node->getParent(), BLACK);
					setColor(temp->getRight(), BLACK);
					leftRotate(node->getParent());
					node = root;
				}
			} else {
				NodeType *temp = node->getParent()->getLeft();
				if (getColor(temp) == RED) {
					setColor(temp, BLACK);
					setColor(node->getParent(), RED);
					rightRotate(node->getParent());
					temp = node->getParent()->getLeft();
				}
				if (getColor(temp->getRight()) == BLACK
						&& getColor(temp->getLeft()) == BLACK) {
					setColor(temp, RED);
					node = node->getParent();
				} else {
					if (getColor(temp->getLeft()) == BLACK) {
						setColor(temp->getRight(), BLACK);
						setColor(temp, RED);
						leftRotate(temp);
						temp = node->getParent()->getLeft();
					}
					setColor(temp, getColor(node->getParent()));
					setColor(node->getParent(), BLACK);
					setColor(temp->getLeft(), BLACK);
					rightRotate(node->getParent());
					node = root;
				}
			}
		}
		setColor(node, BLACK);
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
	void exchangeValues(NodeType *n, NodeType *successor) {
		const T tempValue = successor->getValue();
		successor->setValue(n->getValue());
		n->setValue(tempValue);
		postExchangeValues(n, successor);
	}

private:
	int (*compare)(const T&, const T&);
	NodeType *root;
	uint32_t size;

	inline void setColor(NodeType *node, NodeColor color) {
		if (node != NULL) {
			node->setColor(color);
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
	inline NodeColor getColor(NodeType *node) {
		return node == NULL ? BLACK : node->getColor();
	}

	inline NodeType* getFirstNodeImpl() const {
		NodeType *result = root;
		if (result != NULL) {
			while (result->getLeft() != NULL) {
				result = result->getLeft();
			}
		}
		return result;
	}

	inline NodeType* getLastNodeImpl() const {
		NodeType *result = root;
		if (result != NULL) {
			while (result->getRight() != NULL) {
				result = result->getRight();
			}
		}
		return result;
	}

	inline NodeType* getNodeImpl(const T &value) const {
		NodeType *node = root;
		while (node != NULL) {
			int delta = compare(node->getValue(), value);
			if (delta < 0) {
				node = node->getRight();
			} else if (delta > 0) {
				node = node->getLeft();
			} else {
				break;
			}
		}
		return node;
	}

	inline NodeType* predecessor(const NodeType *node) const {
		if (node == NULL) {
			return NULL;
		}
		NodeType *result = const_cast<NodeType*>(node);
		if (result->getLeft() != NULL) {
			result = result->getLeft();
			while (result->getRight() != NULL)
				result = result->getRight();
			return result;
		}
		NodeType *temp = result->getParent();
		while (temp != NULL && result == temp->getLeft()) {
			result = temp;
			temp = temp->getParent();
		}
		return temp;
	}

	inline NodeType* getPredecessorImpl(const Node<T> *node) const {
		return predecessor(node);
	}

	inline NodeType* getPredecessorImpl(const LinkedNode<T> *node) const {
		return const_cast<NodeType*>(node)->getPredecessor();
	}

	inline NodeType* successor(const NodeType *node) const {
		if (node == NULL) {
			return NULL;
		}
		NodeType *result = const_cast<NodeType*>(node);
		if (result->getRight() != NULL) {
			result = result->getRight();
			while (result->getLeft() != NULL)
				result = result->getLeft();
			return result;
		}
		NodeType *temp = result->getParent();
		while (temp != NULL && node == temp->getRight()) {
			node = temp;
			temp = temp->getParent();
		}
		return temp;
	}

	inline NodeType* getSuccessorImpl(const Node<T> *node) const {
		return successor(node);
	}

	inline NodeType* getSuccessorImpl(const LinkedNode<T> *node) const {
		return const_cast<NodeType*>(node)->getSuccessor();
	}

	inline void postInsert(Node<T> *node) {
		// no op
	}

	inline void postInsert(LinkedNode<T> *linkedNode) {
		NodeType *pred = predecessor(linkedNode);
		linkedNode->setPredecessor(pred);
		if (pred != NULL) {
			pred->setSuccessor(linkedNode);
		}
		NodeType *succ = successor(linkedNode);
		linkedNode->setSuccessor(succ);
		if (succ != NULL) {
			succ->setPredecessor(linkedNode);
		}
	}

	inline void postDelete(Node<T> *node) {
		// no op
	}

	inline void postDelete(LinkedNode<T> *linkedNode) {
		if (linkedNode->getPredecessor() != NULL)
			linkedNode->getPredecessor()->setSuccessor(
					linkedNode->getSuccessor());
		if (linkedNode->getSuccessor() != NULL)
			linkedNode->getSuccessor()->setPredecessor(
					linkedNode->getPredecessor());
	}

	inline void postExchangeValues(Node<T> *n, Node<T> *successor) {
		// no op
	}

	inline void postExchangeValues(LinkedNode<T> *linkedNode,
			LinkedNode<T> *linkedSuccessor) {
		linkedNode->setSuccessor(linkedSuccessor->getSuccessor());
		if (linkedNode->getSuccessor() != NULL)
			linkedNode->getSuccessor()->setPredecessor(linkedNode);
		linkedSuccessor->setPredecessor(NULL);
		linkedSuccessor->setSuccessor(NULL);
	}
};

template<class T>
class Node {
public:
	Node(T value) :
			color(BLACK), left(NULL), right(NULL), parent(NULL), value(value) {
	}

	NodeColor getColor() const {
		return color;
	}

	Node* getLeft() {
		return left;
	}

	const Node* getLeft() const {
		return left;
	}

	Node* getRight() {
		return right;
	}

	const Node* getRight() const {
		return right;
	}

	Node* getParent() {
		return parent;
	}

	const Node* getParent() const {
		return parent;
	}

	const T& getValue() const {
		return value;
	}

	bool isLeaf() const {
		return left == NULL && right == NULL;
	}

private:
	NodeColor color;
	Node *left, *right, *parent;
	T value;

	void setLeft(Node *node) {
		left = node;
	}

	void setRight(Node *node) {
		right = node;
	}

	void setParent(Node *node) {
		parent = node;
	}

	void setValue(const T &value) {
		this->value = value;
	}

	void setColor(NodeColor color) {
		this->color = color;
	}

	friend class RedBlackTree<T, Node> ;
};

template<class T>
class LinkedNode {
public:
	LinkedNode(T value) :
			color(BLACK), left(NULL), right(NULL), parent(NULL), value(value), successor(
					NULL), predecessor(NULL) {
	}

	NodeColor getColor() const {
		return color;
	}

	LinkedNode* getLeft() {
		return left;
	}

	const LinkedNode* getLeft() const {
		return left;
	}

	LinkedNode* getRight() {
		return right;
	}

	const LinkedNode* getRight() const {
		return right;
	}

	LinkedNode* getParent() {
		return parent;
	}

	const LinkedNode* getParent() const {
		return parent;
	}

	const T& getValue() const {
		return value;
	}

	bool isLeaf() const {
		return left == NULL && right == NULL;
	}

	LinkedNode* getSuccessor() {
		return successor;
	}

	const LinkedNode* getSuccessor() const {
		return successor;
	}

	LinkedNode* getPredecessor() {
		return predecessor;
	}

	const LinkedNode* getPredecessor() const {
		return predecessor;
	}

private:
	NodeColor color;
	LinkedNode *left, *right, *parent;
	T value;
	LinkedNode *successor, *predecessor;

	void setLeft(LinkedNode *node) {
		left = node;
	}

	void setRight(LinkedNode *node) {
		right = node;
	}

	void setParent(LinkedNode *node) {
		parent = node;
	}

	void setValue(const T &value) {
		this->value = value;
	}

	void setColor(NodeColor color) {
		this->color = color;
	}

	void setSuccessor(LinkedNode *node) {
		successor = node;
	}

	void setPredecessor(LinkedNode *node) {
		predecessor = node;
	}

	friend class RedBlackTree<T, LinkedNode> ;
};

#endif /* RED_BLACK_TREE_H_ */
