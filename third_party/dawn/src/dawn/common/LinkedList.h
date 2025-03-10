// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is a copy of Chromium's /src/base/containers/linked_list.h with the following
// modifications:
//   - Added iterators for ranged based iterations
//   - Added in list check before removing node to prevent segfault, now returns true iff removed
//   - Added MoveInto functionality for moving list elements to another list

#ifndef SRC_DAWN_COMMON_LINKEDLIST_H_
#define SRC_DAWN_COMMON_LINKEDLIST_H_

#include "dawn/common/Assert.h"
#include "partition_alloc/pointers/raw_ptr_exclusion.h"

namespace dawn {

// Simple LinkedList type. (See the Q&A section to understand how this
// differs from std::list).
//
// To use, start by declaring the class which will be contained in the linked
// list, as extending LinkNode (this gives it next/previous pointers).
//
//   class MyNodeType : public LinkNode<MyNodeType> {
//     ...
//   };
//
// Next, to keep track of the list's head/tail, use a LinkedList instance:
//
//   LinkedList<MyNodeType> list;
//
// To add elements to the list, use any of LinkedList::Append,
// LinkNode::InsertBefore, or LinkNode::InsertAfter:
//
//   LinkNode<MyNodeType>* n1 = ...;
//   LinkNode<MyNodeType>* n2 = ...;
//   LinkNode<MyNodeType>* n3 = ...;
//
//   list.Append(n1);
//   list.Append(n3);
//   n3->InsertBefore(n3);
//
// Lastly, to iterate through the linked list forwards:
//
//   for (LinkNode<MyNodeType>* node = list.head();
//        node != list.end();
//        node = node->next()) {
//     MyNodeType* value = node->value();
//     ...
//   }
//
//   for (LinkNode<MyNodeType*> node : list) {
//     MyNodeType* value = node->value();
//     ...
//   }
//
// Or to iterate the linked list backwards:
//
//   for (LinkNode<MyNodeType>* node = list.tail();
//        node != list.end();
//        node = node->previous()) {
//     MyNodeType* value = node->value();
//     ...
//   }
//
// Questions and Answers:
//
// Q. Should I use std::list or base::LinkedList?
//
// A. The main reason to use base::LinkedList over std::list is
//    performance. If you don't care about the performance differences
//    then use an STL container, as it makes for better code readability.
//
//    Comparing the performance of base::LinkedList<T> to std::list<T*>:
//
//    * Erasing an element of type T* from base::LinkedList<T> is
//      an O(1) operation. Whereas for std::list<T*> it is O(n).
//      That is because with std::list<T*> you must obtain an
//      iterator to the T* element before you can call erase(iterator).
//
//    * Insertion operations with base::LinkedList<T> never require
//      heap allocations.
//
// Q. How does base::LinkedList implementation differ from std::list?
//
// A. Doubly-linked lists are made up of nodes that contain "next" and
//    "previous" pointers that reference other nodes in the list.
//
//    With base::LinkedList<T>, the type being inserted already reserves
//    space for the "next" and "previous" pointers (base::LinkNode<T>*).
//    Whereas with std::list<T> the type can be anything, so the implementation
//    needs to glue on the "next" and "previous" pointers using
//    some internal node type.

// Forward declarations of the types in order for recursive referencing and friending.
template <typename T>
class LinkNode;
template <typename T>
class LinkedList;

template <typename T>
class LinkNode {
  public:
    LinkNode() : previous_(nullptr), next_(nullptr) {}
    LinkNode(LinkNode<T>* previous, LinkNode<T>* next) : previous_(previous), next_(next) {}

    LinkNode(LinkNode<T>&& rhs) {
        next_ = rhs.next_;
        rhs.next_ = nullptr;
        previous_ = rhs.previous_;
        rhs.previous_ = nullptr;

        // If the node belongs to a list, next_ and previous_ are both non-null.
        // Otherwise, they are both null.
        if (next_) {
            next_->previous_ = this;
            previous_->next_ = this;
        }
    }

    ~LinkNode() {
        // Remove the node from any list, otherwise there can be outstanding references to the node
        // even after it has been freed.
        RemoveFromList();
    }

    // Insert |this| into the linked list, before |e|.
    void InsertBefore(LinkNode<T>* e) {
        this->next_ = e;
        this->previous_ = e->previous_;
        e->previous_->next_ = this;
        e->previous_ = this;
    }

    // Insert |this| into the linked list, after |e|.
    void InsertAfter(LinkNode<T>* e) {
        this->next_ = e->next_;
        this->previous_ = e;
        e->next_->previous_ = this;
        e->next_ = this;
    }

    // Check if |this| is in a list.
    bool IsInList() const {
        DAWN_ASSERT((this->previous_ == nullptr) == (this->next_ == nullptr));
        return this->next_ != nullptr;
    }

    // Remove |this| from the linked list. Returns true iff removed from a list.
    bool RemoveFromList() {
        if (!IsInList()) {
            return false;
        }

        this->previous_->next_ = this->next_;
        this->next_->previous_ = this->previous_;
        // next() and previous() return non-null if and only this node is not in any list.
        this->next_ = nullptr;
        this->previous_ = nullptr;
        return true;
    }

    LinkNode<T>* previous() const { return previous_; }

    LinkNode<T>* next() const { return next_; }

    // Cast from the node-type to the value type.
    const T* value() const { return static_cast<const T*>(this); }

    T* value() { return static_cast<T*>(this); }

  private:
    friend class LinkedList<T>;
    // RAW_PTR_EXCLUSION: Linked lists are used and iterated a lot so these pointers are very hot.
    // All accesses to the pointers are behind "safe" APIs such that it is not possible (in
    // single-threaded code) to use them after they are freed.
    RAW_PTR_EXCLUSION LinkNode<T>* previous_ = nullptr;
    RAW_PTR_EXCLUSION LinkNode<T>* next_ = nullptr;
};

template <typename T>
class LinkedList {
  public:
    // The "root" node is self-referential, and forms the basis of a circular
    // list (root_.next() will point back to the start of the list,
    // and root_->previous() wraps around to the end of the list).
    LinkedList() : root_(&root_, &root_) {}

    // Appends |e| to the end of the linked list.
    void Append(LinkNode<T>* e) { e->InsertBefore(&root_); }

    // Prepends |e| to the front og the linked list.
    void Prepend(LinkNode<T>* e) { e->InsertAfter(&root_); }

    // Moves all elements (in order) of the list and appends them into |l| leaving the list empty.
    void MoveInto(LinkedList<T>* l) {
        if (empty()) {
            return;
        }
        l->root_.previous_->next_ = root_.next_;
        root_.next_->previous_ = l->root_.previous_;
        l->root_.previous_ = root_.previous_;
        root_.previous_->next_ = &l->root_;

        root_.next_ = &root_;
        root_.previous_ = &root_;
    }

    LinkNode<T>* head() const { return root_.next(); }

    LinkNode<T>* tail() const { return root_.previous(); }

    const LinkNode<T>* end() const { return &root_; }

    bool empty() const { return head() == end(); }

  private:
    LinkNode<T> root_;
};

template <typename T>
class LinkedListIterator {
  public:
    explicit LinkedListIterator(LinkNode<T>* node) : current_(node), next_(node->next()) {}

    // We keep an early reference to the next node in the list so that even if the current element
    // is modified or removed from the list, we have a valid next node.
    LinkedListIterator<T> const& operator++() {
        current_ = next_;
        next_ = current_->next();
        return *this;
    }

    bool operator!=(const LinkedListIterator<T>& other) const { return current_ != other.current_; }

    LinkNode<T>* operator*() const { return current_; }

  private:
    // RAW_PTR_EXCLUSION: Linked lists are used and iterated a lot so these pointers are very hot.
    // All accesses to the pointers are behind "safe" APIs such that it is not possible (in
    // single-threaded code) to use them after they are freed.
    RAW_PTR_EXCLUSION LinkNode<T>* current_ = nullptr;
    RAW_PTR_EXCLUSION LinkNode<T>* next_ = nullptr;
};

template <typename T>
LinkedListIterator<T> begin(LinkedList<T>& l) {
    return LinkedListIterator<T>(l.head());
}

// Free end function does't use LinkedList<T>::end because of it's const nature. Instead we wrap
// around from tail.
template <typename T>
LinkedListIterator<T> end(LinkedList<T>& l) {
    return LinkedListIterator<T>(l.tail()->next());
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_LINKEDLIST_H_
