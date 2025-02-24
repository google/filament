//===- subzero/src/IceThreading.h - Threading functions ---------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares threading-related functions.
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICETHREADING_H
#define SUBZERO_SRC_ICETHREADING_H

#include "IceDefs.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

namespace Ice {

/// BoundedProducerConsumerQueue is a work queue that allows multiple producers
/// and multiple consumers. A producer adds entries using blockingPush(), and
/// may block if the queue is "full". A producer uses notifyEnd() to indicate
/// that no more entries will be added. A consumer removes an item using
/// blockingPop(), which will return nullptr if notifyEnd() has been called and
/// the queue is empty (it never returns nullptr if the queue contained any
/// items).
///
/// The MaxSize ctor arg controls the maximum size the queue can grow to
/// (subject to a hard limit of MaxStaticSize-1). The Sequential arg indicates
/// purely sequential execution in which the single thread should never wait().
///
/// Two condition variables are used in the implementation. GrewOrEnded signals
/// a waiting worker that a producer has changed the state of the queue. Shrunk
/// signals a blocked producer that a consumer has changed the state of the
/// queue.
///
/// The methods begin with Sequential-specific code to be most clear. The lock
/// and condition variables are not used in the Sequential case.
///
/// Internally, the queue is implemented as a circular array of size
/// MaxStaticSize, where the queue boundaries are denoted by the Front and Back
/// fields. Front==Back indicates an empty queue.
template <typename T, size_t MaxStaticSize = 128>
class BoundedProducerConsumerQueue {
  BoundedProducerConsumerQueue() = delete;
  BoundedProducerConsumerQueue(const BoundedProducerConsumerQueue &) = delete;
  BoundedProducerConsumerQueue &
  operator=(const BoundedProducerConsumerQueue &) = delete;

public:
  BoundedProducerConsumerQueue(bool Sequential, size_t MaxSize = MaxStaticSize)
      : MaxSize(std::min(MaxSize, MaxStaticSize)), Sequential(Sequential) {}
  void blockingPush(std::unique_ptr<T> Item) {
    {
      std::unique_lock<GlobalLockType> L(Lock);
      // If the work queue is already "full", wait for a consumer to grab an
      // element and shrink the queue.
      Shrunk.wait(L, [this] { return size() < MaxSize || Sequential; });
      push(std::move(Item));
    }
    GrewOrEnded.notify_one();
  }
  std::unique_ptr<T> blockingPop(size_t NotifyWhenDownToSize = MaxStaticSize) {
    std::unique_ptr<T> Item;
    bool ShouldNotifyProducer = false;
    {
      std::unique_lock<GlobalLockType> L(Lock);
      GrewOrEnded.wait(L, [this] { return IsEnded || !empty() || Sequential; });
      if (!empty()) {
        Item = pop();
        ShouldNotifyProducer = (size() < NotifyWhenDownToSize) && !IsEnded;
      }
    }
    if (ShouldNotifyProducer)
      Shrunk.notify_one();
    return Item;
  }
  void notifyEnd() {
    {
      std::lock_guard<GlobalLockType> L(Lock);
      IsEnded = true;
    }
    GrewOrEnded.notify_all();
  }

private:
  const static size_t MaxStaticSizeMask = MaxStaticSize - 1;
  static_assert(!(MaxStaticSize & (MaxStaticSize - 1)),
                "MaxStaticSize must be a power of 2");

  ICE_CACHELINE_BOUNDARY;
  /// WorkItems and Lock are read/written by all.
  std::unique_ptr<T> WorkItems[MaxStaticSize];
  ICE_CACHELINE_BOUNDARY;
  /// Lock guards access to WorkItems, Front, Back, and IsEnded.
  GlobalLockType Lock;

  ICE_CACHELINE_BOUNDARY;
  /// GrewOrEnded is written by the producers and read by the consumers. It is
  /// notified (by the producer) when something is added to the queue, in case
  /// consumers are waiting for a non-empty queue.
  std::condition_variable GrewOrEnded;
  /// Back is the index into WorkItems[] of where the next element will be
  /// pushed. (More precisely, Back&MaxStaticSize is the index.) It is written
  /// by the producers, and read by all via size() and empty().
  size_t Back = 0;

  ICE_CACHELINE_BOUNDARY;
  /// Shrunk is notified (by the consumer) when something is removed from the
  /// queue, in case a producer is waiting for the queue to drop below maximum
  /// capacity. It is written by the consumers and read by the producers.
  std::condition_variable Shrunk;
  /// Front is the index into WorkItems[] of the oldest element, i.e. the next
  /// to be popped. (More precisely Front&MaxStaticSize is the index.) It is
  /// written by the consumers, and read by all via size() and empty().
  size_t Front = 0;

  ICE_CACHELINE_BOUNDARY;

  /// MaxSize and Sequential are read by all and written by none.
  const size_t MaxSize;
  const bool Sequential;
  /// IsEnded is read by the consumers, and only written once by the producer.
  bool IsEnded = false;

  /// The lock must be held when the following methods are called.
  bool empty() const { return Front == Back; }
  size_t size() const { return Back - Front; }
  void push(std::unique_ptr<T> Item) {
    WorkItems[Back++ & MaxStaticSizeMask] = std::move(Item);
    assert(size() <= MaxStaticSize);
  }
  std::unique_ptr<T> pop() {
    assert(!empty());
    return std::move(WorkItems[Front++ & MaxStaticSizeMask]);
  }
};

/// EmitterWorkItem is a simple wrapper around a pointer that represents a work
/// item to be emitted, i.e. a function or a set of global declarations and
/// initializers, and it includes a sequence number so that work items can be
/// emitted in a particular order for deterministic output. It acts like an
/// interface class, but instead of making the classes of interest inherit from
/// EmitterWorkItem, it wraps pointers to these classes. Some space is wasted
/// compared to storing the pointers in a union, but not too much due to the
/// work granularity.
class EmitterWorkItem {
  EmitterWorkItem() = delete;
  EmitterWorkItem(const EmitterWorkItem &) = delete;
  EmitterWorkItem &operator=(const EmitterWorkItem &) = delete;

public:
  /// ItemKind can be one of the following:
  ///
  /// WI_Nop: No actual work. This is a placeholder to maintain sequence numbers
  /// in case there is a translation error.
  ///
  /// WI_GlobalInits: A list of global declarations and initializers.
  ///
  /// WI_Asm: A function that has already had emitIAS() called on it. The work
  /// is transferred via the Assembler buffer, and the originating Cfg has been
  /// deleted (to recover lots of memory).
  ///
  /// WI_Cfg: A Cfg that has not yet had emit() or emitIAS() called on it. This
  /// is only used as a debugging configuration when we want to emit "readable"
  /// assembly code, possibly annotated with liveness and other information only
  /// available in the Cfg and not in the Assembler buffer.
  enum ItemKind { WI_Nop, WI_GlobalInits, WI_Asm, WI_Cfg };
  /// Constructor for a WI_Nop work item.
  explicit EmitterWorkItem(uint32_t Seq);
  /// Constructor for a WI_GlobalInits work item.
  EmitterWorkItem(uint32_t Seq, std::unique_ptr<VariableDeclarationList> D);
  /// Constructor for a WI_Asm work item.
  EmitterWorkItem(uint32_t Seq, std::unique_ptr<Assembler> A);
  /// Constructor for a WI_Cfg work item.
  EmitterWorkItem(uint32_t Seq, std::unique_ptr<Cfg> F);
  uint32_t getSequenceNumber() const { return Sequence; }
  ItemKind getKind() const { return Kind; }
  void setGlobalInits(std::unique_ptr<VariableDeclarationList> GloblInits);
  std::unique_ptr<VariableDeclarationList> getGlobalInits();
  std::unique_ptr<Assembler> getAsm();
  std::unique_ptr<Cfg> getCfg();

private:
  const uint32_t Sequence;
  const ItemKind Kind;
  std::unique_ptr<VariableDeclarationList> GlobalInits;
  std::unique_ptr<Assembler> Function;
  std::unique_ptr<Cfg> RawFunc;
};

} // end of namespace Ice

#endif // SUBZERO_SRC_ICETHREADING_H
