// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is a copy of Chromium's /src/base/containers/linked_list_unittest.cc

#include <list>
#include <utility>

#include "dawn/common/LinkedList.h"
#include "gtest/gtest.h"

namespace dawn {
namespace {

class Node : public LinkNode<Node> {
  public:
    explicit Node(int id) : id_(id) {}

    int id() const { return id_; }

    void set_id(int id) { id_ = id; }

  private:
    int id_;
};

class MultipleInheritanceNodeBase {
  public:
    MultipleInheritanceNodeBase() : field_taking_up_space_(0) {}
    int field_taking_up_space_;
};

class MultipleInheritanceNode : public MultipleInheritanceNodeBase,
                                public LinkNode<MultipleInheritanceNode> {
  public:
    MultipleInheritanceNode() = default;
};

class MovableNode : public LinkNode<MovableNode> {
  public:
    explicit MovableNode(int id) : id_(id) {}

    MovableNode(MovableNode&&) = default;

    int id() const { return id_; }

  private:
    int id_;
};

// Checks that when iterating |list| (either from head to tail, or from
// tail to head, as determined by |forward|), we get back |node_ids|,
// which is an array of size |num_nodes|.
void ExpectListContentsForDirection(const LinkedList<Node>& list,
                                    int num_nodes,
                                    const int* node_ids,
                                    bool forward) {
    int i = 0;
    for (const LinkNode<Node>* node = (forward ? list.head() : list.tail()); node != list.end();
         node = (forward ? node->next() : node->previous())) {
        ASSERT_LT(i, num_nodes);
        int index_of_id = forward ? i : num_nodes - i - 1;
        EXPECT_EQ(node_ids[index_of_id], node->value()->id());
        ++i;
    }
    EXPECT_EQ(num_nodes, i);
}

void ExpectListContents(const LinkedList<Node>& list, int num_nodes, const int* node_ids) {
    {
        SCOPED_TRACE("Iterating forward (from head to tail)");
        ExpectListContentsForDirection(list, num_nodes, node_ids, true);
    }
    {
        SCOPED_TRACE("Iterating backward (from tail to head)");
        ExpectListContentsForDirection(list, num_nodes, node_ids, false);
    }
}

TEST(LinkedList, Empty) {
    LinkedList<Node> list;
    EXPECT_EQ(list.end(), list.head());
    EXPECT_EQ(list.end(), list.tail());
    ExpectListContents(list, 0, nullptr);
}

TEST(LinkedList, Append) {
    LinkedList<Node> list;
    ExpectListContents(list, 0, nullptr);

    Node n1(1);
    list.Append(&n1);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n1, list.tail());
    {
        const int expected[] = {1};
        ExpectListContents(list, 1, expected);
    }

    Node n2(2);
    list.Append(&n2);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n2, list.tail());
    {
        const int expected[] = {1, 2};
        ExpectListContents(list, 2, expected);
    }

    Node n3(3);
    list.Append(&n3);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n3, list.tail());
    {
        const int expected[] = {1, 2, 3};
        ExpectListContents(list, 3, expected);
    }
}

TEST(LinkedList, Prepend) {
    LinkedList<Node> list;
    ExpectListContents(list, 0, nullptr);

    Node n1(1);
    list.Prepend(&n1);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n1, list.tail());
    {
        const int expected[] = {1};
        ExpectListContents(list, 1, expected);
    }

    Node n2(2);
    list.Prepend(&n2);

    EXPECT_EQ(&n2, list.head());
    EXPECT_EQ(&n1, list.tail());
    {
        const int expected[] = {2, 1};
        ExpectListContents(list, 2, expected);
    }

    Node n3(3);
    list.Prepend(&n3);

    EXPECT_EQ(&n3, list.head());
    EXPECT_EQ(&n1, list.tail());
    {
        const int expected[] = {3, 2, 1};
        ExpectListContents(list, 3, expected);
    }
}

TEST(LinkedList, RemoveFromList) {
    LinkedList<Node> list;

    Node n1(1);
    Node n2(2);
    Node n3(3);
    Node n4(4);
    Node n5(5);

    list.Append(&n1);
    list.Append(&n2);
    list.Append(&n3);
    list.Append(&n4);
    list.Append(&n5);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n5, list.tail());
    {
        const int expected[] = {1, 2, 3, 4, 5};
        ExpectListContents(list, 5, expected);
    }

    // Remove from the middle.
    n3.RemoveFromList();

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n5, list.tail());
    {
        const int expected[] = {1, 2, 4, 5};
        ExpectListContents(list, 4, expected);
    }

    // Remove from the tail.
    n5.RemoveFromList();

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n4, list.tail());
    {
        const int expected[] = {1, 2, 4};
        ExpectListContents(list, 3, expected);
    }

    // Remove from the head.
    n1.RemoveFromList();

    EXPECT_EQ(&n2, list.head());
    EXPECT_EQ(&n4, list.tail());
    {
        const int expected[] = {2, 4};
        ExpectListContents(list, 2, expected);
    }

    // Empty the list.
    n2.RemoveFromList();
    n4.RemoveFromList();

    ExpectListContents(list, 0, nullptr);
    EXPECT_EQ(list.end(), list.head());
    EXPECT_EQ(list.end(), list.tail());

    // Fill the list once again.
    list.Append(&n1);
    list.Append(&n2);
    list.Append(&n3);
    list.Append(&n4);
    list.Append(&n5);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n5, list.tail());
    {
        const int expected[] = {1, 2, 3, 4, 5};
        ExpectListContents(list, 5, expected);
    }
}

TEST(LinkedList, InsertBefore) {
    LinkedList<Node> list;

    Node n1(1);
    Node n2(2);
    Node n3(3);
    Node n4(4);

    list.Append(&n1);
    list.Append(&n2);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n2, list.tail());
    {
        const int expected[] = {1, 2};
        ExpectListContents(list, 2, expected);
    }

    n3.InsertBefore(&n2);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n2, list.tail());
    {
        const int expected[] = {1, 3, 2};
        ExpectListContents(list, 3, expected);
    }

    n4.InsertBefore(&n1);

    EXPECT_EQ(&n4, list.head());
    EXPECT_EQ(&n2, list.tail());
    {
        const int expected[] = {4, 1, 3, 2};
        ExpectListContents(list, 4, expected);
    }
}

TEST(LinkedList, InsertAfter) {
    LinkedList<Node> list;

    Node n1(1);
    Node n2(2);
    Node n3(3);
    Node n4(4);

    list.Append(&n1);
    list.Append(&n2);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n2, list.tail());
    {
        const int expected[] = {1, 2};
        ExpectListContents(list, 2, expected);
    }

    n3.InsertAfter(&n2);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n3, list.tail());
    {
        const int expected[] = {1, 2, 3};
        ExpectListContents(list, 3, expected);
    }

    n4.InsertAfter(&n1);

    EXPECT_EQ(&n1, list.head());
    EXPECT_EQ(&n3, list.tail());
    {
        const int expected[] = {1, 4, 2, 3};
        ExpectListContents(list, 4, expected);
    }
}

TEST(LinkedList, MultipleInheritanceNode) {
    MultipleInheritanceNode node;
    EXPECT_EQ(&node, node.value());
}

TEST(LinkedList, EmptyListIsEmpty) {
    LinkedList<Node> list;
    EXPECT_TRUE(list.empty());
}

TEST(LinkedList, NonEmptyListIsNotEmpty) {
    LinkedList<Node> list;

    Node n(1);
    list.Append(&n);

    EXPECT_FALSE(list.empty());
}

TEST(LinkedList, EmptiedListIsEmptyAgain) {
    LinkedList<Node> list;

    Node n(1);
    list.Append(&n);
    n.RemoveFromList();

    EXPECT_TRUE(list.empty());
}

TEST(LinkedList, NodesCanBeReused) {
    LinkedList<Node> list1;
    LinkedList<Node> list2;

    Node n(1);
    list1.Append(&n);
    n.RemoveFromList();
    list2.Append(&n);

    EXPECT_EQ(list2.head()->value(), &n);
}

TEST(LinkedList, RemovedNodeHasNullNextPrevious) {
    LinkedList<Node> list;

    Node n(1);
    list.Append(&n);
    n.RemoveFromList();

    EXPECT_EQ(nullptr, n.next());
    EXPECT_EQ(nullptr, n.previous());
}

TEST(LinkedList, NodeMoveConstructor) {
    LinkedList<MovableNode> list;

    MovableNode n1(1);
    MovableNode n2(2);
    MovableNode n3(3);

    list.Append(&n1);
    list.Append(&n2);
    list.Append(&n3);

    EXPECT_EQ(&n1, n2.previous());
    EXPECT_EQ(&n2, n1.next());
    EXPECT_EQ(&n3, n2.next());
    EXPECT_EQ(&n2, n3.previous());
    EXPECT_EQ(2, n2.id());

    MovableNode n2_new(std::move(n2));

    EXPECT_EQ(&n1, n2_new.previous());
    EXPECT_EQ(&n2_new, n1.next());
    EXPECT_EQ(&n3, n2_new.next());
    EXPECT_EQ(&n2_new, n3.previous());
    EXPECT_EQ(2, n2_new.id());
}

TEST(LinkedList, IsInList) {
    LinkedList<Node> list;

    Node n(1);

    EXPECT_FALSE(n.IsInList());
    list.Append(&n);
    EXPECT_TRUE(n.IsInList());
    EXPECT_TRUE(n.RemoveFromList());
    EXPECT_FALSE(n.IsInList());
    EXPECT_FALSE(n.RemoveFromList());
}

TEST(LinkedList, MoveInto) {
    LinkedList<Node> l1;
    LinkedList<Node> l2;

    Node n1(1);
    Node n2(2);
    l1.Append(&n1);
    l2.Append(&n2);

    l2.MoveInto(&l1);
    const int expected[] = {1, 2};
    ExpectListContents(l1, 2, expected);
    EXPECT_TRUE(l2.empty());
}

TEST(LinkedList, MoveEmptyListInto) {
    LinkedList<Node> l1;
    LinkedList<Node> l2;

    Node n1(1);
    Node n2(2);
    l1.Append(&n1);
    l1.Append(&n2);

    l2.MoveInto(&l1);
    const int expected[] = {1, 2};
    ExpectListContents(l1, 2, expected);
    EXPECT_TRUE(l2.empty());
}

TEST(LinkedList, MoveIntoEmpty) {
    LinkedList<Node> l1;
    LinkedList<Node> l2;

    Node n1(1);
    Node n2(2);
    l2.Append(&n1);
    l2.Append(&n2);

    l2.MoveInto(&l1);
    const int expected[] = {1, 2};
    ExpectListContents(l1, 2, expected);
    EXPECT_TRUE(l2.empty());
}

TEST(LinkedList, RangeBasedModify) {
    LinkedList<Node> list;

    Node n1(1);
    Node n2(2);
    list.Append(&n1);
    list.Append(&n2);

    for (LinkNode<Node>* node : list) {
        node->value()->set_id(node->value()->id() + 1);
    }
    const int expected[] = {2, 3};
    ExpectListContents(list, 2, expected);
}

TEST(LinkedList, RangeBasedEndIsEnd) {
    LinkedList<Node> list;
    EXPECT_EQ(list.end(), *end(list));
}

}  // anonymous namespace
}  // namespace dawn
