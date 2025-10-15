# Binary Search Tree synchronization

# Overview

This project implements a thread-safe integer Binary Search Tree (BST) library in C.
The goal is to support concurrent operations such as insertion, deletion, and search from multiple threads without data corruption or invalid reads.

# Features
* Create, insert, delete, and search nodes in a BST.

* Thread-safe design using fine-grained locking to allow concurrent read/write access where possible.

* Supports in-order, pre-order, and post-order traversals.

* Safe memory deallocation for all nodes.

# Synchronization Design

* Each TreeNode includes an internal mutex lock to protect its children pointers during updates.

* Operations such as insertNode and deleteNode use lock coupling (hand-over-hand locking) to minimize contention.

* Read operations (searchNode, findMin) acquire only the necessary read locks, allowing multiple concurrent readers.

* Write operations (insert/delete) lock affected subtrees only, not the entire structure.

* Ensures data consistency

