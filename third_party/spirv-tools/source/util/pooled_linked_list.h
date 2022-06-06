// Copyright (c) 2021 The Khronos Group Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SOURCE_UTIL_POOLED_LINKED_LIST_H_
#define SOURCE_UTIL_POOLED_LINKED_LIST_H_

#include <cstdint>
#include <vector>

namespace spvtools {
namespace utils {

// Shared storage of nodes for PooledLinkedList.
template <typename T>
class PooledLinkedListNodes {
 public:
  struct Node {
    Node(T e, int32_t n = -1) : element(e), next(n) {}

    T element = {};
    int32_t next = -1;
  };

  PooledLinkedListNodes() = default;
  PooledLinkedListNodes(const PooledLinkedListNodes&) = delete;
  PooledLinkedListNodes& operator=(const PooledLinkedListNodes&) = delete;

  PooledLinkedListNodes(PooledLinkedListNodes&& that) {
    *this = std::move(that);
  }

  PooledLinkedListNodes& operator=(PooledLinkedListNodes&& that) {
    vec_ = std::move(that.vec_);
    free_nodes_ = that.free_nodes_;
    return *this;
  }

  size_t total_nodes() { return vec_.size(); }
  size_t free_nodes() { return free_nodes_; }
  size_t used_nodes() { return total_nodes() - free_nodes(); }

 private:
  template <typename ListT>
  friend class PooledLinkedList;

  Node& at(int32_t index) { return vec_[index]; }
  const Node& at(int32_t index) const { return vec_[index]; }

  int32_t insert(T element) {
    int32_t index = int32_t(vec_.size());
    vec_.emplace_back(element);
    return index;
  }

  std::vector<Node> vec_;
  size_t free_nodes_ = 0;
};

// Implements a linked-list where list nodes come from a shared pool. This is
// meant to be used in scenarios where it is desirable to avoid many small
// allocations.
//
// Instead of pointers, the list uses indices to allow the underlying storage
// to be modified without needing to modify the list. When removing elements
// from the list, nodes are not deleted or recycled: to reclaim unused space,
// perform a sequence of |move_nodes| operations into a temporary pool, which
// then is moved into the old pool.
//
// This does *not* attempt to implement a full stl-compatible interface.
template <typename T>
class PooledLinkedList {
 public:
  using NodePool = PooledLinkedListNodes<T>;
  using Node = typename NodePool::Node;

  PooledLinkedList() = delete;
  PooledLinkedList(NodePool* nodes) : nodes_(nodes) {}

  // Shared iterator implementation (for iterator and const_iterator).
  template <typename ElementT, typename PoolT>
  class iterator_base {
   public:
    iterator_base(const iterator_base& i)
        : nodes_(i.nodes_), index_(i.index_) {}

    iterator_base& operator++() {
      index_ = nodes_->at(index_).next;
      return *this;
    }

    iterator_base& operator=(const iterator_base& i) {
      nodes_ = i.nodes_;
      index_ = i.index_;
      return *this;
    }

    ElementT& operator*() const { return nodes_->at(index_).element; }
    ElementT* operator->() const { return &nodes_->at(index_).element; }

    friend inline bool operator==(const iterator_base& lhs,
                                  const iterator_base& rhs) {
      return lhs.nodes_ == rhs.nodes_ && lhs.index_ == rhs.index_;
    }
    friend inline bool operator!=(const iterator_base& lhs,
                                  const iterator_base& rhs) {
      return lhs.nodes_ != rhs.nodes_ || lhs.index_ != rhs.index_;
    }

    // Define standard iterator types needs so this class can be
    // used with <algorithms>.
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = ElementT;
    using pointer = ElementT*;
    using const_pointer = const ElementT*;
    using reference = ElementT&;
    using const_reference = const ElementT&;
    using size_type = size_t;

   private:
    friend PooledLinkedList;

    iterator_base(PoolT* pool, int32_t index) : nodes_(pool), index_(index) {}

    PoolT* nodes_;
    int32_t index_ = -1;
  };

  using iterator = iterator_base<T, std::vector<Node>>;
  using const_iterator = iterator_base<const T, const std::vector<Node>>;

  bool empty() const { return head_ == -1; }

  T& front() { return nodes_->at(head_).element; }
  T& back() { return nodes_->at(tail_).element; }
  const T& front() const { return nodes_->at(head_).element; }
  const T& back() const { return nodes_->at(tail_).element; }

  iterator begin() { return iterator(&nodes_->vec_, head_); }
  iterator end() { return iterator(&nodes_->vec_, -1); }
  const_iterator begin() const { return const_iterator(&nodes_->vec_, head_); }
  const_iterator end() const { return const_iterator(&nodes_->vec_, -1); }

  // Inserts |element| at the back of the list.
  void push_back(T element) {
    int32_t new_tail = nodes_->insert(element);
    if (head_ == -1) {
      head_ = new_tail;
      tail_ = new_tail;
    } else {
      nodes_->at(tail_).next = new_tail;
      tail_ = new_tail;
    }
  }

  // Removes the first occurrence of |element| from the list.
  // Returns if |element| was removed.
  bool remove_first(T element) {
    int32_t* prev_next = &head_;
    for (int32_t prev_index = -1, index = head_; index != -1; /**/) {
      auto& node = nodes_->at(index);
      if (node.element == element) {
        // Snip from of the list, optionally fixing up tail pointer.
        if (tail_ == index) {
          assert(node.next == -1);
          tail_ = prev_index;
        }
        *prev_next = node.next;
        nodes_->free_nodes_++;
        return true;
      } else {
        prev_next = &node.next;
      }
      prev_index = index;
      index = node.next;
    }
    return false;
  }

  // Returns the PooledLinkedListNodes that owns this list's nodes.
  NodePool* pool() { return nodes_; }

  // Moves the nodes in this list into |new_pool|, providing a way to compact
  // storage and reclaim unused space.
  //
  // Upon completing a sequence of |move_nodes| calls, you must ensure you
  // retain ownership of the new storage your lists point to. Example usage:
  //
  //    unique_ptr<NodePool> new_pool = ...;
  //    for (PooledLinkedList& list : lists) {
  //        list.move_to(new_pool);
  //    }
  //    my_pool_ = std::move(new_pool);
  void move_nodes(NodePool* new_pool) {
    // Be sure to construct the list in the same order, instead of simply
    // doing a sequence of push_backs.
    int32_t prev_entry = -1;
    int32_t nodes_freed = 0;
    for (int32_t index = head_; index != -1; nodes_freed++) {
      const auto& node = nodes_->at(index);
      int32_t this_entry = new_pool->insert(node.element);
      index = node.next;
      if (prev_entry == -1) {
        head_ = this_entry;
      } else {
        new_pool->at(prev_entry).next = this_entry;
      }
      prev_entry = this_entry;
    }
    tail_ = prev_entry;
    // Update our old pool's free count, now we're a member of the new pool.
    nodes_->free_nodes_ += nodes_freed;
    nodes_ = new_pool;
  }

 private:
  NodePool* nodes_;
  int32_t head_ = -1;
  int32_t tail_ = -1;
};

}  // namespace utils
}  // namespace spvtools

#endif  // SOURCE_UTIL_POOLED_LINKED_LIST_H_