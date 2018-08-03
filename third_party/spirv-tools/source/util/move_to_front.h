// Copyright (c) 2017 Google Inc.
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

#ifndef LIBSPIRV_UTIL_MOVE_TO_FRONT_H_
#define LIBSPIRV_UTIL_MOVE_TO_FRONT_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace spvtools {
namespace utils {

// Log(n) move-to-front implementation. Implements the following functions:
// Insert - pushes value to the front of the mtf sequence
//          (only unique values allowed).
// Remove - remove value from the sequence.
// ValueFromRank - access value by its 1-indexed rank in the sequence.
// RankFromValue - get the rank of the given value in the sequence.
// Accessing a value with ValueFromRank or RankFromValue moves the value to the
// front of the sequence (rank of 1).
//
// The implementation is based on an AVL-based order statistic tree. The tree
// is ordered by timestamps issued when values are inserted or accessed (recent
// values go to the left side of the tree, old values are gradually rotated to
// the right side).
//
// Terminology
// rank: 1-indexed rank showing how recently the value was inserted or accessed.
// node: handle used internally to access node data.
// size: size of the subtree of a node (including the node).
// height: distance from a node to the farthest leaf.
template <typename Val>
class MoveToFront {
 public:
  explicit MoveToFront(size_t reserve_capacity = 4) {
    nodes_.reserve(reserve_capacity);

    // Create NIL node.
    nodes_.emplace_back(Node());
  }

  virtual ~MoveToFront() {}

  // Inserts value in the move-to-front sequence. Does nothing if the value is
  // already in the sequence. Returns true if insertion was successful.
  // The inserted value is placed at the front of the sequence (rank 1).
  bool Insert(const Val& value);

  // Removes value from move-to-front sequence. Returns false iff the value
  // was not found.
  bool Remove(const Val& value);

  // Computes 1-indexed rank of value in the move-to-front sequence and moves
  // the value to the front. Example:
  // Before the call: 4 8 2 1 7
  // RankFromValue(8) returns 2
  // After the call: 8 4 2 1 7
  // Returns true iff the value was found in the sequence.
  bool RankFromValue(const Val& value, uint32_t* rank);

  // Returns value corresponding to a 1-indexed rank in the move-to-front
  // sequence and moves the value to the front. Example:
  // Before the call: 4 8 2 1 7
  // ValueFromRank(2) returns 8
  // After the call: 8 4 2 1 7
  // Returns true iff the rank is within bounds [1, GetSize()].
  bool ValueFromRank(uint32_t rank, Val* value);

  // Moves the value to the front of the sequence.
  // Returns false iff value is not in the sequence.
  bool Promote(const Val& value);

  // Returns true iff the move-to-front sequence contains the value.
  bool HasValue(const Val& value) const;

  // Returns the number of elements in the move-to-front sequence.
  uint32_t GetSize() const { return SizeOf(root_); }

 protected:
  // Internal tree data structure uses handles instead of pointers. Leaves and
  // root parent reference a singleton under handle 0. Although dereferencing
  // a null pointer is not possible, inappropriate access to handle 0 would
  // cause an assertion. Handles are not garbage collected if value was
  // deprecated
  // with DeprecateValue(). But handles are recycled when a node is
  // repositioned.

  // Internal tree data structure node.
  struct Node {
    // Timestamp from a logical clock which updates every time the element is
    // accessed through ValueFromRank or RankFromValue.
    uint32_t timestamp = 0;
    // The size of the node's subtree, including the node.
    // SizeOf(LeftOf(node)) + SizeOf(RightOf(node)) + 1.
    uint32_t size = 0;
    // Handles to connected nodes.
    uint32_t left = 0;
    uint32_t right = 0;
    uint32_t parent = 0;
    // Distance to the farthest leaf.
    // Leaves have height 0, real nodes at least 1.
    uint32_t height = 0;
    // Stored value.
    Val value = Val();
  };

  // Creates node and sets correct values. Non-NIL nodes should be created only
  // through this function. If the node with this value has been created
  // previously
  // and since orphaned, reuses the old node instead of creating a new one.
  uint32_t CreateNode(uint32_t timestamp, const Val& value) {
    uint32_t handle = static_cast<uint32_t>(nodes_.size());
    const auto result = value_to_node_.emplace(value, handle);
    if (result.second) {
      // Create new node.
      nodes_.emplace_back(Node());
      Node& node = nodes_.back();
      node.timestamp = timestamp;
      node.value = value;
      node.size = 1;
      // Non-NIL nodes start with height 1 because their NIL children are
      // leaves.
      node.height = 1;
    } else {
      // Reuse old node.
      handle = result.first->second;
      assert(!IsInTree(handle));
      assert(ValueOf(handle) == value);
      assert(SizeOf(handle) == 1);
      assert(HeightOf(handle) == 1);
      MutableTimestampOf(handle) = timestamp;
    }

    return handle;
  }

  // Node accessor methods. Naming is designed to be similar to natural
  // language as these functions tend to be used in sequences, for example:
  // ParentOf(LeftestDescendentOf(RightOf(node)))

  // Returns value of the node referenced by |handle|.
  Val ValueOf(uint32_t node) const { return nodes_.at(node).value; }

  // Returns left child of |node|.
  uint32_t LeftOf(uint32_t node) const { return nodes_.at(node).left; }

  // Returns right child of |node|.
  uint32_t RightOf(uint32_t node) const { return nodes_.at(node).right; }

  // Returns parent of |node|.
  uint32_t ParentOf(uint32_t node) const { return nodes_.at(node).parent; }

  // Returns timestamp of |node|.
  uint32_t TimestampOf(uint32_t node) const {
    assert(node);
    return nodes_.at(node).timestamp;
  }

  // Returns size of |node|.
  uint32_t SizeOf(uint32_t node) const { return nodes_.at(node).size; }

  // Returns height of |node|.
  uint32_t HeightOf(uint32_t node) const { return nodes_.at(node).height; }

  // Returns mutable reference to value of |node|.
  Val& MutableValueOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).value;
  }

  // Returns mutable reference to handle of left child of |node|.
  uint32_t& MutableLeftOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).left;
  }

  // Returns mutable reference to handle of right child of |node|.
  uint32_t& MutableRightOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).right;
  }

  // Returns mutable reference to handle of parent of |node|.
  uint32_t& MutableParentOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).parent;
  }

  // Returns mutable reference to timestamp of |node|.
  uint32_t& MutableTimestampOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).timestamp;
  }

  // Returns mutable reference to size of |node|.
  uint32_t& MutableSizeOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).size;
  }

  // Returns mutable reference to height of |node|.
  uint32_t& MutableHeightOf(uint32_t node) {
    assert(node);
    return nodes_.at(node).height;
  }

  // Returns true iff |node| is left child of its parent.
  bool IsLeftChild(uint32_t node) const {
    assert(node);
    return LeftOf(ParentOf(node)) == node;
  }

  // Returns true iff |node| is right child of its parent.
  bool IsRightChild(uint32_t node) const {
    assert(node);
    return RightOf(ParentOf(node)) == node;
  }

  // Returns true iff |node| has no relatives.
  bool IsOrphan(uint32_t node) const {
    assert(node);
    return !ParentOf(node) && !LeftOf(node) && !RightOf(node);
  }

  // Returns true iff |node| is in the tree.
  bool IsInTree(uint32_t node) const {
    assert(node);
    return node == root_ || !IsOrphan(node);
  }

  // Returns the height difference between right and left subtrees.
  int BalanceOf(uint32_t node) const {
    return int(HeightOf(RightOf(node))) - int(HeightOf(LeftOf(node)));
  }

  // Updates size and height of the node, assuming that the children have
  // correct values.
  void UpdateNode(uint32_t node);

  // Returns the most LeftOf(LeftOf(... descendent which is not leaf.
  uint32_t LeftestDescendantOf(uint32_t node) const {
    uint32_t parent = 0;
    while (node) {
      parent = node;
      node = LeftOf(node);
    }
    return parent;
  }

  // Returns the most RightOf(RightOf(... descendent which is not leaf.
  uint32_t RightestDescendantOf(uint32_t node) const {
    uint32_t parent = 0;
    while (node) {
      parent = node;
      node = RightOf(node);
    }
    return parent;
  }

  // Inserts node in the tree. The node must be an orphan.
  void InsertNode(uint32_t node);

  // Removes node from the tree. May change value_to_node_ if removal uses a
  // scapegoat. Returns the removed (orphaned) handle for recycling. The
  // returned handle may not be equal to |node| if scapegoat was used.
  uint32_t RemoveNode(uint32_t node);

  // Rotates |node| left, reassigns all connections and returns the node
  // which takes place of the |node|.
  uint32_t RotateLeft(const uint32_t node);

  // Rotates |node| right, reassigns all connections and returns the node
  // which takes place of the |node|.
  uint32_t RotateRight(const uint32_t node);

  // Root node handle. The tree is empty if root_ is 0.
  uint32_t root_ = 0;

  // Incremented counters for next timestamp and value.
  uint32_t next_timestamp_ = 1;

  // Holds all tree nodes. Indices of this vector are node handles.
  std::vector<Node> nodes_;

  // Maps ids to node handles.
  std::unordered_map<Val, uint32_t> value_to_node_;

  // Cache for the last accessed value in the sequence.
  Val last_accessed_value_ = Val();
  bool last_accessed_value_valid_ = false;
};

template <typename Val>
class MultiMoveToFront {
 public:
  // Inserts |value| to sequence with handle |mtf|.
  // Returns false if |mtf| already has |value|.
  bool Insert(uint64_t mtf, const Val& value) {
    if (GetMtf(mtf).Insert(value)) {
      val_to_mtfs_[value].insert(mtf);
      return true;
    }
    return false;
  }

  // Removes |value| from sequence with handle |mtf|.
  // Returns false if |mtf| doesn't have |value|.
  bool Remove(uint64_t mtf, const Val& value) {
    if (GetMtf(mtf).Remove(value)) {
      val_to_mtfs_[value].erase(mtf);
      return true;
    }
    assert(val_to_mtfs_[value].count(mtf) == 0);
    return false;
  }

  // Removes |value| from all sequences which have it.
  void RemoveFromAll(const Val& value) {
    auto it = val_to_mtfs_.find(value);
    if (it == val_to_mtfs_.end()) return;

    auto& mtfs_containing_value = it->second;
    for (uint64_t mtf : mtfs_containing_value) {
      GetMtf(mtf).Remove(value);
    }

    val_to_mtfs_.erase(value);
  }

  // Computes rank of |value| in sequence |mtf|.
  // Returns false if |mtf| doesn't have |value|.
  bool RankFromValue(uint64_t mtf, const Val& value, uint32_t* rank) {
    return GetMtf(mtf).RankFromValue(value, rank);
  }

  // Finds |value| with |rank| in sequence |mtf|.
  // Returns false if |rank| is out of bounds.
  bool ValueFromRank(uint64_t mtf, uint32_t rank, Val* value) {
    return GetMtf(mtf).ValueFromRank(rank, value);
  }

  // Returns size of |mtf| sequence.
  uint32_t GetSize(uint64_t mtf) { return GetMtf(mtf).GetSize(); }

  // Promotes |value| in all sequences which have it.
  void Promote(const Val& value) {
    const auto it = val_to_mtfs_.find(value);
    if (it == val_to_mtfs_.end()) return;

    const auto& mtfs_containing_value = it->second;
    for (uint64_t mtf : mtfs_containing_value) {
      GetMtf(mtf).Promote(value);
    }
  }

  // Inserts |value| in sequence |mtf| or promotes if it's already there.
  void InsertOrPromote(uint64_t mtf, const Val& value) {
    if (!Insert(mtf, value)) {
      GetMtf(mtf).Promote(value);
    }
  }

  // Returns if |mtf| sequence has |value|.
  bool HasValue(uint64_t mtf, const Val& value) {
    return GetMtf(mtf).HasValue(value);
  }

 private:
  // Returns actual MoveToFront object corresponding to |handle|.
  // As multiple operations are often performed consecutively for the same
  // sequence, the last returned value is cached.
  MoveToFront<Val>& GetMtf(uint64_t handle) {
    if (!cached_mtf_ || cached_handle_ != handle) {
      cached_handle_ = handle;
      cached_mtf_ = &mtfs_[handle];
    }

    return *cached_mtf_;
  }

  // Container holding MoveToFront objects. Map key is sequence handle.
  std::map<uint64_t, MoveToFront<Val>> mtfs_;

  // Container mapping value to sequences which contain that value.
  std::unordered_map<Val, std::set<uint64_t>> val_to_mtfs_;

  // Cache for the last accessed sequence.
  uint64_t cached_handle_ = 0;
  MoveToFront<Val>* cached_mtf_ = nullptr;
};

template <typename Val>
bool MoveToFront<Val>::Insert(const Val& value) {
  auto it = value_to_node_.find(value);
  if (it != value_to_node_.end() && IsInTree(it->second)) return false;

  const uint32_t old_size = GetSize();
  (void)old_size;

  InsertNode(CreateNode(next_timestamp_++, value));

  last_accessed_value_ = value;
  last_accessed_value_valid_ = true;

  assert(value_to_node_.count(value));
  assert(old_size + 1 == GetSize());
  return true;
}

template <typename Val>
bool MoveToFront<Val>::Remove(const Val& value) {
  auto it = value_to_node_.find(value);
  if (it == value_to_node_.end()) return false;

  if (!IsInTree(it->second)) return false;

  if (last_accessed_value_ == value) last_accessed_value_valid_ = false;

  const uint32_t orphan = RemoveNode(it->second);
  (void)orphan;
  // The node of |value| is still alive but it's orphaned now. Can still be
  // reused later.
  assert(!IsInTree(orphan));
  assert(ValueOf(orphan) == value);
  return true;
}

template <typename Val>
bool MoveToFront<Val>::RankFromValue(const Val& value, uint32_t* rank) {
  if (last_accessed_value_valid_ && last_accessed_value_ == value) {
    *rank = 1;
    return true;
  }

  const uint32_t old_size = GetSize();
  if (old_size == 1) {
    if (ValueOf(root_) == value) {
      *rank = 1;
      return true;
    } else {
      return false;
    }
  }

  const auto it = value_to_node_.find(value);
  if (it == value_to_node_.end()) {
    return false;
  }

  uint32_t target = it->second;

  if (!IsInTree(target)) {
    return false;
  }

  uint32_t node = target;
  *rank = 1 + SizeOf(LeftOf(node));
  while (node) {
    if (IsRightChild(node)) *rank += 1 + SizeOf(LeftOf(ParentOf(node)));
    node = ParentOf(node);
  }

  // Don't update timestamp if the node has rank 1.
  if (*rank != 1) {
    // Update timestamp and reposition the node.
    target = RemoveNode(target);
    assert(ValueOf(target) == value);
    assert(old_size == GetSize() + 1);
    MutableTimestampOf(target) = next_timestamp_++;
    InsertNode(target);
    assert(old_size == GetSize());
  }

  last_accessed_value_ = value;
  last_accessed_value_valid_ = true;
  return true;
}

template <typename Val>
bool MoveToFront<Val>::HasValue(const Val& value) const {
  const auto it = value_to_node_.find(value);
  if (it == value_to_node_.end()) {
    return false;
  }

  return IsInTree(it->second);
}

template <typename Val>
bool MoveToFront<Val>::Promote(const Val& value) {
  if (last_accessed_value_valid_ && last_accessed_value_ == value) {
    return true;
  }

  const uint32_t old_size = GetSize();
  if (old_size == 1) return ValueOf(root_) == value;

  const auto it = value_to_node_.find(value);
  if (it == value_to_node_.end()) {
    return false;
  }

  uint32_t target = it->second;

  if (!IsInTree(target)) {
    return false;
  }

  // Update timestamp and reposition the node.
  target = RemoveNode(target);
  assert(ValueOf(target) == value);
  assert(old_size == GetSize() + 1);
  MutableTimestampOf(target) = next_timestamp_++;
  InsertNode(target);
  assert(old_size == GetSize());

  last_accessed_value_ = value;
  last_accessed_value_valid_ = true;
  return true;
}

template <typename Val>
bool MoveToFront<Val>::ValueFromRank(uint32_t rank, Val* value) {
  if (last_accessed_value_valid_ && rank == 1) {
    *value = last_accessed_value_;
    return true;
  }

  const uint32_t old_size = GetSize();
  if (rank <= 0 || rank > old_size) {
    return false;
  }

  if (old_size == 1) {
    *value = ValueOf(root_);
    return true;
  }

  const bool update_timestamp = (rank != 1);

  uint32_t node = root_;
  while (node) {
    const uint32_t left_subtree_num_nodes = SizeOf(LeftOf(node));
    if (rank == left_subtree_num_nodes + 1) {
      // This is the node we are looking for.
      // Don't update timestamp if the node has rank 1.
      if (update_timestamp) {
        node = RemoveNode(node);
        assert(old_size == GetSize() + 1);
        MutableTimestampOf(node) = next_timestamp_++;
        InsertNode(node);
        assert(old_size == GetSize());
      }
      *value = ValueOf(node);
      last_accessed_value_ = *value;
      last_accessed_value_valid_ = true;
      return true;
    }

    if (rank < left_subtree_num_nodes + 1) {
      // Descend into the left subtree. The rank is still valid.
      node = LeftOf(node);
    } else {
      // Descend into the right subtree. We leave behind the left subtree and
      // the current node, adjust the |rank| accordingly.
      rank -= left_subtree_num_nodes + 1;
      node = RightOf(node);
    }
  }

  assert(0);
  return false;
}

template <typename Val>
void MoveToFront<Val>::InsertNode(uint32_t node) {
  assert(!IsInTree(node));
  assert(SizeOf(node) == 1);
  assert(HeightOf(node) == 1);
  assert(TimestampOf(node));

  if (!root_) {
    root_ = node;
    return;
  }

  uint32_t iter = root_;
  uint32_t parent = 0;

  // Will determine if |node| will become the right or left child after
  // insertion (but before balancing).
  bool right_child = true;

  // Find the node which will become |node|'s parent after insertion
  // (but before balancing).
  while (iter) {
    parent = iter;
    assert(TimestampOf(iter) != TimestampOf(node));
    right_child = TimestampOf(iter) > TimestampOf(node);
    iter = right_child ? RightOf(iter) : LeftOf(iter);
  }

  assert(parent);

  // Connect node and parent.
  MutableParentOf(node) = parent;
  if (right_child)
    MutableRightOf(parent) = node;
  else
    MutableLeftOf(parent) = node;

  // Insertion is finished. Start the balancing process.
  bool needs_rebalancing = true;
  parent = ParentOf(node);

  while (parent) {
    UpdateNode(parent);

    if (needs_rebalancing) {
      const int parent_balance = BalanceOf(parent);

      if (RightOf(parent) == node) {
        // Added node to the right subtree.
        if (parent_balance > 1) {
          // Parent is right heavy, rotate left.
          if (BalanceOf(node) < 0) RotateRight(node);
          parent = RotateLeft(parent);
        } else if (parent_balance == 0 || parent_balance == -1) {
          // Parent is balanced or left heavy, no need to balance further.
          needs_rebalancing = false;
        }
      } else {
        // Added node to the left subtree.
        if (parent_balance < -1) {
          // Parent is left heavy, rotate right.
          if (BalanceOf(node) > 0) RotateLeft(node);
          parent = RotateRight(parent);
        } else if (parent_balance == 0 || parent_balance == 1) {
          // Parent is balanced or right heavy, no need to balance further.
          needs_rebalancing = false;
        }
      }
    }

    assert(BalanceOf(parent) >= -1 && (BalanceOf(parent) <= 1));

    node = parent;
    parent = ParentOf(parent);
  }
}

template <typename Val>
uint32_t MoveToFront<Val>::RemoveNode(uint32_t node) {
  if (LeftOf(node) && RightOf(node)) {
    // If |node| has two children, then use another node as scapegoat and swap
    // their contents. We pick the scapegoat on the side of the tree which has
    // more nodes.
    const uint32_t scapegoat = SizeOf(LeftOf(node)) >= SizeOf(RightOf(node))
                                   ? RightestDescendantOf(LeftOf(node))
                                   : LeftestDescendantOf(RightOf(node));
    assert(scapegoat);
    std::swap(MutableValueOf(node), MutableValueOf(scapegoat));
    std::swap(MutableTimestampOf(node), MutableTimestampOf(scapegoat));
    value_to_node_[ValueOf(node)] = node;
    value_to_node_[ValueOf(scapegoat)] = scapegoat;
    node = scapegoat;
  }

  // |node| may have only one child at this point.
  assert(!RightOf(node) || !LeftOf(node));

  uint32_t parent = ParentOf(node);
  uint32_t child = RightOf(node) ? RightOf(node) : LeftOf(node);

  // Orphan |node| and reconnect parent and child.
  if (child) MutableParentOf(child) = parent;

  if (parent) {
    if (LeftOf(parent) == node)
      MutableLeftOf(parent) = child;
    else
      MutableRightOf(parent) = child;
  }

  MutableParentOf(node) = 0;
  MutableLeftOf(node) = 0;
  MutableRightOf(node) = 0;
  UpdateNode(node);
  const uint32_t orphan = node;

  if (root_ == node) root_ = child;

  // Removal is finished. Start the balancing process.
  bool needs_rebalancing = true;
  node = child;

  while (parent) {
    UpdateNode(parent);

    if (needs_rebalancing) {
      const int parent_balance = BalanceOf(parent);

      if (parent_balance == 1 || parent_balance == -1) {
        // The height of the subtree was not changed.
        needs_rebalancing = false;
      } else {
        if (RightOf(parent) == node) {
          // Removed node from the right subtree.
          if (parent_balance < -1) {
            // Parent is left heavy, rotate right.
            const uint32_t sibling = LeftOf(parent);
            if (BalanceOf(sibling) > 0) RotateLeft(sibling);
            parent = RotateRight(parent);
          }
        } else {
          // Removed node from the left subtree.
          if (parent_balance > 1) {
            // Parent is right heavy, rotate left.
            const uint32_t sibling = RightOf(parent);
            if (BalanceOf(sibling) < 0) RotateRight(sibling);
            parent = RotateLeft(parent);
          }
        }
      }
    }

    assert(BalanceOf(parent) >= -1 && (BalanceOf(parent) <= 1));

    node = parent;
    parent = ParentOf(parent);
  }

  return orphan;
}

template <typename Val>
uint32_t MoveToFront<Val>::RotateLeft(const uint32_t node) {
  const uint32_t pivot = RightOf(node);
  assert(pivot);

  // LeftOf(pivot) gets attached to node in place of pivot.
  MutableRightOf(node) = LeftOf(pivot);
  if (RightOf(node)) MutableParentOf(RightOf(node)) = node;

  // Pivot gets attached to ParentOf(node) in place of node.
  MutableParentOf(pivot) = ParentOf(node);
  if (!ParentOf(node))
    root_ = pivot;
  else if (IsLeftChild(node))
    MutableLeftOf(ParentOf(node)) = pivot;
  else
    MutableRightOf(ParentOf(node)) = pivot;

  // Node is child of pivot.
  MutableLeftOf(pivot) = node;
  MutableParentOf(node) = pivot;

  // Update both node and pivot. Pivot is the new parent of node, so node should
  // be updated first.
  UpdateNode(node);
  UpdateNode(pivot);

  return pivot;
}

template <typename Val>
uint32_t MoveToFront<Val>::RotateRight(const uint32_t node) {
  const uint32_t pivot = LeftOf(node);
  assert(pivot);

  // RightOf(pivot) gets attached to node in place of pivot.
  MutableLeftOf(node) = RightOf(pivot);
  if (LeftOf(node)) MutableParentOf(LeftOf(node)) = node;

  // Pivot gets attached to ParentOf(node) in place of node.
  MutableParentOf(pivot) = ParentOf(node);
  if (!ParentOf(node))
    root_ = pivot;
  else if (IsLeftChild(node))
    MutableLeftOf(ParentOf(node)) = pivot;
  else
    MutableRightOf(ParentOf(node)) = pivot;

  // Node is child of pivot.
  MutableRightOf(pivot) = node;
  MutableParentOf(node) = pivot;

  // Update both node and pivot. Pivot is the new parent of node, so node should
  // be updated first.
  UpdateNode(node);
  UpdateNode(pivot);

  return pivot;
}

template <typename Val>
void MoveToFront<Val>::UpdateNode(uint32_t node) {
  MutableSizeOf(node) = 1 + SizeOf(LeftOf(node)) + SizeOf(RightOf(node));
  MutableHeightOf(node) =
      1 + std::max(HeightOf(LeftOf(node)), HeightOf(RightOf(node)));
}

}  // namespace utils
}  // namespace spvtools

#endif  // LIBSPIRV_UTIL_MOVE_TO_FRONT_H_
