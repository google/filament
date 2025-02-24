// Copyright 2020 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// marl::DAG<> provides an ahead of time, declarative, directed acyclic
// task graph.

#ifndef marl_dag_h
#define marl_dag_h

#include "containers.h"
#include "export.h"
#include "memory.h"
#include "scheduler.h"
#include "waitgroup.h"

namespace marl {
namespace detail {
using DAGCounter = std::atomic<uint32_t>;
template <typename T>
struct DAGRunContext {
  T data;
  Allocator::unique_ptr<DAGCounter> counters;

  template <typename F>
  MARL_NO_EXPORT inline void invoke(F&& f) {
    f(data);
  }
};
template <>
struct DAGRunContext<void> {
  Allocator::unique_ptr<DAGCounter> counters;

  template <typename F>
  MARL_NO_EXPORT inline void invoke(F&& f) {
    f();
  }
};
template <typename T>
struct DAGWork {
  using type = std::function<void(T)>;
};
template <>
struct DAGWork<void> {
  using type = std::function<void()>;
};
}  // namespace detail

///////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class DAG;

template <typename T>
class DAGBuilder;

template <typename T>
class DAGNodeBuilder;

///////////////////////////////////////////////////////////////////////////////
// DAGBase<T>
///////////////////////////////////////////////////////////////////////////////

// DAGBase is derived by DAG<T> and DAG<void>. It has no public API.
template <typename T>
class DAGBase {
 protected:
  friend DAGBuilder<T>;
  friend DAGNodeBuilder<T>;

  using RunContext = detail::DAGRunContext<T>;
  using Counter = detail::DAGCounter;
  using NodeIndex = size_t;
  using Work = typename detail::DAGWork<T>::type;
  static const constexpr size_t NumReservedNodes = 32;
  static const constexpr size_t NumReservedNumOuts = 4;
  static const constexpr size_t InvalidCounterIndex = ~static_cast<size_t>(0);
  static const constexpr NodeIndex RootIndex = 0;
  static const constexpr NodeIndex InvalidNodeIndex =
      ~static_cast<NodeIndex>(0);

  // DAG work node.
  struct Node {
    MARL_NO_EXPORT inline Node() = default;
    MARL_NO_EXPORT inline Node(Work&& work);
    MARL_NO_EXPORT inline Node(const Work& work);

    // The work to perform for this node in the graph.
    Work work;

    // counterIndex if valid, is the index of the counter in the RunContext for
    // this node. The counter is decremented for each completed dependency task
    // (ins), and once it reaches 0, this node will be invoked.
    size_t counterIndex = InvalidCounterIndex;

    // Indices for all downstream nodes.
    containers::vector<NodeIndex, NumReservedNumOuts> outs;
  };

  // initCounters() allocates and initializes the ctx->coutners from
  // initialCounters.
  MARL_NO_EXPORT inline void initCounters(RunContext* ctx,
                                          Allocator* allocator);

  // notify() is called each time a dependency task (ins) has completed for the
  // node with the given index.
  // If all dependency tasks have completed (or this is the root node) then
  // notify() returns true and the caller should then call invoke().
  MARL_NO_EXPORT inline bool notify(RunContext*, NodeIndex);

  // invoke() calls the work function for the node with the given index, then
  // calls notify() and possibly invoke() for all the dependee nodes.
  MARL_NO_EXPORT inline void invoke(RunContext*, NodeIndex, WaitGroup*);

  // nodes is the full list of the nodes in the graph.
  // nodes[0] is always the root node, which has no dependencies (ins).
  containers::vector<Node, NumReservedNodes> nodes;

  // initialCounters is a list of initial counter values to be copied to
  // RunContext::counters on DAG<>::run().
  // initialCounters is indexed by Node::counterIndex, and only contains counts
  // for nodes that have at least 2 dependencies (ins) - because of this the
  // number of entries in initialCounters may be fewer than nodes.
  containers::vector<uint32_t, NumReservedNodes> initialCounters;
};

template <typename T>
DAGBase<T>::Node::Node(Work&& work) : work(std::move(work)) {}

template <typename T>
DAGBase<T>::Node::Node(const Work& work) : work(work) {}

template <typename T>
void DAGBase<T>::initCounters(RunContext* ctx, Allocator* allocator) {
  auto numCounters = initialCounters.size();
  ctx->counters = allocator->make_unique_n<Counter>(numCounters);
  for (size_t i = 0; i < numCounters; i++) {
    ctx->counters.get()[i] = {initialCounters[i]};
  }
}

template <typename T>
bool DAGBase<T>::notify(RunContext* ctx, NodeIndex nodeIdx) {
  Node* node = &nodes[nodeIdx];

  // If we have multiple dependencies, decrement the counter and check whether
  // we've reached 0.
  if (node->counterIndex == InvalidCounterIndex) {
    return true;
  }
  auto counters = ctx->counters.get();
  auto counter = --counters[node->counterIndex];
  return counter == 0;
}

template <typename T>
void DAGBase<T>::invoke(RunContext* ctx, NodeIndex nodeIdx, WaitGroup* wg) {
  Node* node = &nodes[nodeIdx];

  // Run this node's work.
  if (node->work) {
    ctx->invoke(node->work);
  }

  // Then call notify() on all dependees (outs), and invoke() those that
  // returned true.
  // We buffer the node to invoke (toInvoke) so we can schedule() all but the
  // last node to invoke(), and directly call the last invoke() on this thread.
  // This is done to avoid the overheads of scheduling when a direct call would
  // suffice.
  NodeIndex toInvoke = InvalidNodeIndex;
  for (NodeIndex idx : node->outs) {
    if (notify(ctx, idx)) {
      if (toInvoke != InvalidNodeIndex) {
        wg->add(1);
        // Schedule while promoting the WaitGroup capture from a pointer
        // reference to a value. This ensures that the WaitGroup isn't dropped
        // while in use.
        schedule(
            [=](WaitGroup wg) {
              invoke(ctx, toInvoke, &wg);
              wg.done();
            },
            *wg);
      }
      toInvoke = idx;
    }
  }
  if (toInvoke != InvalidNodeIndex) {
    invoke(ctx, toInvoke, wg);
  }
}

///////////////////////////////////////////////////////////////////////////////
// DAGNodeBuilder<T>
///////////////////////////////////////////////////////////////////////////////

// DAGNodeBuilder is the builder interface for a DAG node.
template <typename T>
class DAGNodeBuilder {
  using NodeIndex = typename DAGBase<T>::NodeIndex;

 public:
  // then() builds and returns a new DAG node that will be invoked after this
  // node has completed.
  //
  // F is a function that will be called when the new DAG node is invoked, with
  // the signature:
  //   void(T)   when T is not void
  // or
  //   void()    when T is void
  template <typename F>
  MARL_NO_EXPORT inline DAGNodeBuilder then(F&&);

 private:
  friend DAGBuilder<T>;
  MARL_NO_EXPORT inline DAGNodeBuilder(DAGBuilder<T>*, NodeIndex);
  DAGBuilder<T>* builder;
  NodeIndex index;
};

template <typename T>
DAGNodeBuilder<T>::DAGNodeBuilder(DAGBuilder<T>* builder, NodeIndex index)
    : builder(builder), index(index) {}

template <typename T>
template <typename F>
DAGNodeBuilder<T> DAGNodeBuilder<T>::then(F&& work) {
  auto node = builder->node(std::forward<F>(work));
  builder->addDependency(*this, node);
  return node;
}

///////////////////////////////////////////////////////////////////////////////
// DAGBuilder<T>
///////////////////////////////////////////////////////////////////////////////
template <typename T>
class DAGBuilder {
 public:
  // DAGBuilder constructor
  MARL_NO_EXPORT inline DAGBuilder(Allocator* allocator = Allocator::Default);

  // root() returns the root DAG node.
  MARL_NO_EXPORT inline DAGNodeBuilder<T> root();

  // node() builds and returns a new DAG node with no initial dependencies.
  // The returned node must be attached to the graph in order to invoke F or any
  // of the dependees of this returned node.
  //
  // F is a function that will be called when the new DAG node is invoked, with
  // the signature:
  //   void(T)   when T is not void
  // or
  //   void()    when T is void
  template <typename F>
  MARL_NO_EXPORT inline DAGNodeBuilder<T> node(F&& work);

  // node() builds and returns a new DAG node that depends on all the tasks in
  // after to be completed before invoking F.
  //
  // F is a function that will be called when the new DAG node is invoked, with
  // the signature:
  //   void(T)   when T is not void
  // or
  //   void()    when T is void
  template <typename F>
  MARL_NO_EXPORT inline DAGNodeBuilder<T> node(
      F&& work,
      std::initializer_list<DAGNodeBuilder<T>> after);

  // addDependency() adds parent as dependency on child. All dependencies of
  // child must have completed before child is invoked.
  MARL_NO_EXPORT inline void addDependency(DAGNodeBuilder<T> parent,
                                           DAGNodeBuilder<T> child);

  // build() constructs and returns the DAG. No other methods of this class may
  // be called after calling build().
  MARL_NO_EXPORT inline Allocator::unique_ptr<DAG<T>> build();

 private:
  static const constexpr size_t NumReservedNumIns = 4;
  using Node = typename DAG<T>::Node;

  // The DAG being built.
  Allocator::unique_ptr<DAG<T>> dag;

  // Number of dependencies (ins) for each node in dag->nodes.
  containers::vector<uint32_t, NumReservedNumIns> numIns;
};

template <typename T>
DAGBuilder<T>::DAGBuilder(Allocator* allocator /* = Allocator::Default */)
    : dag(allocator->make_unique<DAG<T>>()), numIns(allocator) {
  // Add root
  dag->nodes.emplace_back(Node{});
  numIns.emplace_back(0);
}

template <typename T>
DAGNodeBuilder<T> DAGBuilder<T>::root() {
  return DAGNodeBuilder<T>{this, DAGBase<T>::RootIndex};
}

template <typename T>
template <typename F>
DAGNodeBuilder<T> DAGBuilder<T>::node(F&& work) {
  return node(std::forward<F>(work), {});
}

template <typename T>
template <typename F>
DAGNodeBuilder<T> DAGBuilder<T>::node(
    F&& work,
    std::initializer_list<DAGNodeBuilder<T>> after) {
  MARL_ASSERT(numIns.size() == dag->nodes.size(),
              "NodeBuilder vectors out of sync");
  auto index = dag->nodes.size();
  numIns.emplace_back(0);
  dag->nodes.emplace_back(Node{std::forward<F>(work)});
  auto node = DAGNodeBuilder<T>{this, index};
  for (auto in : after) {
    addDependency(in, node);
  }
  return node;
}

template <typename T>
void DAGBuilder<T>::addDependency(DAGNodeBuilder<T> parent,
                                  DAGNodeBuilder<T> child) {
  numIns[child.index]++;
  dag->nodes[parent.index].outs.push_back(child.index);
}

template <typename T>
Allocator::unique_ptr<DAG<T>> DAGBuilder<T>::build() {
  auto numNodes = dag->nodes.size();
  MARL_ASSERT(numIns.size() == dag->nodes.size(),
              "NodeBuilder vectors out of sync");
  for (size_t i = 0; i < numNodes; i++) {
    if (numIns[i] > 1) {
      auto& node = dag->nodes[i];
      node.counterIndex = dag->initialCounters.size();
      dag->initialCounters.push_back(numIns[i]);
    }
  }
  return std::move(dag);
}

///////////////////////////////////////////////////////////////////////////////
// DAG<T>
///////////////////////////////////////////////////////////////////////////////
template <typename T = void>
class DAG : public DAGBase<T> {
 public:
  using Builder = DAGBuilder<T>;
  using NodeBuilder = DAGNodeBuilder<T>;

  // run() invokes the function of each node in the graph of the DAG, passing
  // data to each, starting with the root node. All dependencies need to have
  // completed their function before dependees will be invoked.
  MARL_NO_EXPORT inline void run(T& data,
                                 Allocator* allocator = Allocator::Default);
};

template <typename T>
void DAG<T>::run(T& arg, Allocator* allocator /* = Allocator::Default */) {
  typename DAGBase<T>::RunContext ctx{arg};
  this->initCounters(&ctx, allocator);
  WaitGroup wg;
  this->invoke(&ctx, this->RootIndex, &wg);
  wg.wait();
}

///////////////////////////////////////////////////////////////////////////////
// DAG<void>
///////////////////////////////////////////////////////////////////////////////
template <>
class DAG<void> : public DAGBase<void> {
 public:
  using Builder = DAGBuilder<void>;
  using NodeBuilder = DAGNodeBuilder<void>;

  // run() invokes the function of each node in the graph of the DAG, starting
  // with the root node. All dependencies need to have completed their function
  // before dependees will be invoked.
  MARL_NO_EXPORT inline void run(Allocator* allocator = Allocator::Default);
};

void DAG<void>::run(Allocator* allocator /* = Allocator::Default */) {
  typename DAGBase<void>::RunContext ctx{};
  this->initCounters(&ctx, allocator);
  WaitGroup wg;
  this->invoke(&ctx, this->RootIndex, &wg);
  wg.wait();
}

}  // namespace marl

#endif  // marl_dag_h
