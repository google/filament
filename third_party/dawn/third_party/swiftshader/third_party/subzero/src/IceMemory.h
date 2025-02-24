//===- subzero/src/IceMemory.h - Memory management declarations -*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Declares some useful data structures and routines dealing with
/// memory management in Subzero (mostly, allocator types.)
///
//===----------------------------------------------------------------------===//

#ifndef SUBZERO_SRC_ICEMEMORY_H
#define SUBZERO_SRC_ICEMEMORY_H

#include "IceTLS.h"

#include "llvm/Support/Allocator.h"

#include <cstddef>
#include <mutex>

namespace Ice {

class Cfg;
class GlobalContext;
class Liveness;

using ArenaAllocator =
    llvm::BumpPtrAllocatorImpl<llvm::MallocAllocator, /*SlabSize=*/1024 * 1024>;

class LockedArenaAllocator {
  LockedArenaAllocator() = delete;
  LockedArenaAllocator(const LockedArenaAllocator &) = delete;
  LockedArenaAllocator &operator=(const LockedArenaAllocator &) = delete;

public:
  LockedArenaAllocator(ArenaAllocator *Alloc, std::mutex *Mutex)
      : Alloc(Alloc), AutoLock(*Mutex) {}
  LockedArenaAllocator(LockedArenaAllocator &&) = default;
  LockedArenaAllocator &operator=(LockedArenaAllocator &&) = default;
  ~LockedArenaAllocator() = default;

  ArenaAllocator *operator->() { return Alloc; }

private:
  ArenaAllocator *Alloc;
  std::unique_lock<std::mutex> AutoLock;
};

template <typename T, typename Traits> struct sz_allocator {
  /// std::allocator interface implementation.
  /// @{
  using value_type = T;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = const T &;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  sz_allocator() : Current() {}
  template <class U>
  sz_allocator(const sz_allocator<U, Traits> &) : Current() {}

  pointer address(reference x) const {
    return reinterpret_cast<pointer>(&reinterpret_cast<char &>(x));
  }
  const_pointer address(const_reference x) const {
    return reinterpret_cast<const_pointer>(&reinterpret_cast<const char &>(x));
  }

  pointer allocate(size_type num) {
    assert(current() != nullptr);
    return current()->template Allocate<T>(num);
  }

  template <typename... A> void construct(pointer P, A &&... Args) {
    new (static_cast<void *>(P)) T(std::forward<A>(Args)...);
  }

  void deallocate(pointer, size_type) {}

  template <class U> struct rebind { typedef sz_allocator<U, Traits> other; };

  void destroy(pointer P) { P->~T(); }
  /// @}

  /// Manages the current underlying allocator.
  /// @{
  typename Traits::allocator_type current() {
    if (!Traits::cache_allocator) {
      // TODO(jpp): allocators should always be cacheable... maybe. Investigate.
      return Traits::current();
    }
    if (Current == nullptr) {
      Current = Traits::current();
    }
    assert(Current == Traits::current());
    return Current;
  }
  static void init() { Traits::init(); }
  /// @}
  typename Traits::allocator_type Current;
};

template <class Traits> struct sz_allocator_scope {
  explicit sz_allocator_scope(typename Traits::manager_type *Manager) {
    Traits::set_current(Manager);
  }

  ~sz_allocator_scope() { Traits::set_current(nullptr); }
};

template <typename T, typename U, typename Traits>
inline bool operator==(const sz_allocator<T, Traits> &,
                       const sz_allocator<U, Traits> &) {
  return true;
}

template <typename T, typename U, typename Traits>
inline bool operator!=(const sz_allocator<T, Traits> &,
                       const sz_allocator<U, Traits> &) {
  return false;
}

class CfgAllocatorTraits {
  CfgAllocatorTraits() = delete;
  CfgAllocatorTraits(const CfgAllocatorTraits &) = delete;
  CfgAllocatorTraits &operator=(const CfgAllocatorTraits &) = delete;
  ~CfgAllocatorTraits() = delete;

public:
  using allocator_type = ArenaAllocator *;
  using manager_type = Cfg;
  static constexpr bool cache_allocator = false;

  static void init() { ICE_TLS_INIT_FIELD(CfgAllocator); }

  static allocator_type current();
  static void set_current(const manager_type *Manager);
  static void set_current(ArenaAllocator *Allocator);
  static void set_current(std::nullptr_t);

private:
  ICE_TLS_DECLARE_FIELD(ArenaAllocator *, CfgAllocator);
};

template <typename T>
using CfgLocalAllocator = sz_allocator<T, CfgAllocatorTraits>;

using CfgLocalAllocatorScope = sz_allocator_scope<CfgAllocatorTraits>;

class LivenessAllocatorTraits {
  LivenessAllocatorTraits() = delete;
  LivenessAllocatorTraits(const LivenessAllocatorTraits &) = delete;
  LivenessAllocatorTraits &operator=(const LivenessAllocatorTraits &) = delete;
  ~LivenessAllocatorTraits() = delete;

public:
  using allocator_type = ArenaAllocator *;
  using manager_type = Liveness;
  static constexpr bool cache_allocator = true;

  static void init() { ICE_TLS_INIT_FIELD(LivenessAllocator); }

  static allocator_type current();
  static void set_current(const manager_type *Manager);

private:
  ICE_TLS_DECLARE_FIELD(ArenaAllocator *, LivenessAllocator);
};

template <typename T>
using LivenessAllocator = sz_allocator<T, LivenessAllocatorTraits>;

using LivenessAllocatorScope = sz_allocator_scope<LivenessAllocatorTraits>;

} // end of namespace Ice

#endif // SUBZERO_SRC_ICEMEMORY_H
