// Copyright 2019 The Marl Authors.
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

#ifndef marl_memory_h
#define marl_memory_h

#include "debug.h"
#include "export.h"

#include <stdint.h>

#include <array>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <utility>  // std::forward

namespace marl {

template <typename T>
struct StlAllocator;

// pageSize() returns the size in bytes of a virtual memory page for the host
// system.
MARL_EXPORT
size_t pageSize();

template <typename T>
MARL_NO_EXPORT inline T alignUp(T val, T alignment) {
  return alignment * ((val + alignment - 1) / alignment);
}

// aligned_storage() is a replacement for std::aligned_storage that isn't busted
// on older versions of MSVC.
template <size_t SIZE, size_t ALIGNMENT>
struct aligned_storage {
  struct alignas(ALIGNMENT) type {
    unsigned char data[SIZE];
  };
};

///////////////////////////////////////////////////////////////////////////////
// Allocation
///////////////////////////////////////////////////////////////////////////////

// Allocation holds the result of a memory allocation from an Allocator.
struct Allocation {
  // Intended usage of the allocation. Used for allocation trackers.
  enum class Usage : uint8_t {
    Undefined = 0,
    Stack,   // Fiber stack
    Create,  // Allocator::create(), make_unique(), make_shared()
    Vector,  // marl::containers::vector<T>
    List,    // marl::containers::list<T>
    Stl,     // marl::StlAllocator
    Count,   // Not intended to be used as a usage type - used for upper bound.
  };

  // Request holds all the information required to make an allocation.
  struct Request {
    size_t size = 0;                 // The size of the allocation in bytes.
    size_t alignment = 0;            // The minimum alignment of the allocation.
    bool useGuards = false;          // Whether the allocation is guarded.
    Usage usage = Usage::Undefined;  // Intended usage of the allocation.
  };

  void* ptr = nullptr;  // The pointer to the allocated memory.
  Request request;      // Request used for the allocation.
};

///////////////////////////////////////////////////////////////////////////////
// Allocator
///////////////////////////////////////////////////////////////////////////////

// Allocator is an interface to a memory allocator.
// Marl provides a default implementation with Allocator::Default.
class Allocator {
 public:
  // The default allocator. Initialized with an implementation that allocates
  // from the OS. Can be assigned a custom implementation.
  MARL_EXPORT static Allocator* Default;

  // Deleter is a smart-pointer compatible deleter that can be used to delete
  // objects created by Allocator::create(). Deleter is used by the smart
  // pointers returned by make_shared() and make_unique().
  struct MARL_EXPORT Deleter {
    MARL_NO_EXPORT inline Deleter();
    MARL_NO_EXPORT inline Deleter(Allocator* allocator, size_t count);

    template <typename T>
    MARL_NO_EXPORT inline void operator()(T* object);

    Allocator* allocator = nullptr;
    size_t count = 0;
  };

  // unique_ptr<T> is an alias to std::unique_ptr<T, Deleter>.
  template <typename T>
  using unique_ptr = std::unique_ptr<T, Deleter>;

  virtual ~Allocator() = default;

  // allocate() allocates memory from the allocator.
  // The returned Allocation::request field must be equal to the Request
  // parameter.
  virtual Allocation allocate(const Allocation::Request&) = 0;

  // free() frees the memory returned by allocate().
  // The Allocation must have all fields equal to those returned by allocate().
  virtual void free(const Allocation&) = 0;

  // create() allocates and constructs an object of type T, respecting the
  // alignment of the type.
  // The pointer returned by create() must be deleted with destroy().
  template <typename T, typename... ARGS>
  inline T* create(ARGS&&... args);

  // destroy() destructs and frees the object allocated with create().
  template <typename T>
  inline void destroy(T* object);

  // make_unique() returns a new object allocated from the allocator wrapped
  // in a unique_ptr that respects the alignment of the type.
  template <typename T, typename... ARGS>
  inline unique_ptr<T> make_unique(ARGS&&... args);

  // make_unique_n() returns an array of n new objects allocated from the
  // allocator wrapped in a unique_ptr that respects the alignment of the
  // type.
  template <typename T, typename... ARGS>
  inline unique_ptr<T> make_unique_n(size_t n, ARGS&&... args);

  // make_shared() returns a new object allocated from the allocator
  // wrapped in a std::shared_ptr that respects the alignment of the type.
  template <typename T, typename... ARGS>
  inline std::shared_ptr<T> make_shared(ARGS&&... args);

 protected:
  Allocator() = default;
};

///////////////////////////////////////////////////////////////////////////////
// Allocator::Deleter
///////////////////////////////////////////////////////////////////////////////
Allocator::Deleter::Deleter() : allocator(nullptr) {}
Allocator::Deleter::Deleter(Allocator* allocator_, size_t count_)
    : allocator(allocator_), count(count_) {}

template <typename T>
void Allocator::Deleter::operator()(T* object) {
  object->~T();

  Allocation allocation;
  allocation.ptr = object;
  allocation.request.size = sizeof(T) * count;
  allocation.request.alignment = alignof(T);
  allocation.request.usage = Allocation::Usage::Create;
  allocator->free(allocation);
}

///////////////////////////////////////////////////////////////////////////////
// Allocator
///////////////////////////////////////////////////////////////////////////////
template <typename T, typename... ARGS>
T* Allocator::create(ARGS&&... args) {
  Allocation::Request request;
  request.size = sizeof(T);
  request.alignment = alignof(T);
  request.usage = Allocation::Usage::Create;

  auto alloc = allocate(request);
  new (alloc.ptr) T(std::forward<ARGS>(args)...);
  return reinterpret_cast<T*>(alloc.ptr);
}

template <typename T>
void Allocator::destroy(T* object) {
  object->~T();

  Allocation alloc;
  alloc.ptr = object;
  alloc.request.size = sizeof(T);
  alloc.request.alignment = alignof(T);
  alloc.request.usage = Allocation::Usage::Create;
  free(alloc);
}

template <typename T, typename... ARGS>
Allocator::unique_ptr<T> Allocator::make_unique(ARGS&&... args) {
  return make_unique_n<T>(1, std::forward<ARGS>(args)...);
}

template <typename T, typename... ARGS>
Allocator::unique_ptr<T> Allocator::make_unique_n(size_t n, ARGS&&... args) {
  if (n == 0) {
    return nullptr;
  }

  Allocation::Request request;
  request.size = sizeof(T) * n;
  request.alignment = alignof(T);
  request.usage = Allocation::Usage::Create;

  auto alloc = allocate(request);
  new (alloc.ptr) T(std::forward<ARGS>(args)...);
  return unique_ptr<T>(reinterpret_cast<T*>(alloc.ptr), Deleter{this, n});
}

template <typename T, typename... ARGS>
std::shared_ptr<T> Allocator::make_shared(ARGS&&... args) {
  Allocation::Request request;
  request.size = sizeof(T);
  request.alignment = alignof(T);
  request.usage = Allocation::Usage::Create;

  auto alloc = allocate(request);
  new (alloc.ptr) T(std::forward<ARGS>(args)...);
  return std::shared_ptr<T>(reinterpret_cast<T*>(alloc.ptr), Deleter{this, 1});
}

///////////////////////////////////////////////////////////////////////////////
// TrackedAllocator
///////////////////////////////////////////////////////////////////////////////

// TrackedAllocator wraps an Allocator to track the allocations made.
class TrackedAllocator : public Allocator {
 public:
  struct UsageStats {
    // Total number of allocations.
    size_t count = 0;
    // total allocation size in bytes (as requested, may be higher due to
    // alignment or guards).
    size_t bytes = 0;
  };

  struct Stats {
    // numAllocations() returns the total number of allocations across all
    // usages for the allocator.
    inline size_t numAllocations() const;

    // bytesAllocated() returns the total number of bytes allocated across all
    // usages for the allocator.
    inline size_t bytesAllocated() const;

    // Statistics per usage.
    std::array<UsageStats, size_t(Allocation::Usage::Count)> byUsage;
  };

  // Constructor that wraps an existing allocator.
  inline TrackedAllocator(Allocator* allocator);

  // stats() returns the current allocator statistics.
  inline Stats stats();

  // Allocator compliance
  inline Allocation allocate(const Allocation::Request&) override;
  inline void free(const Allocation&) override;

 private:
  Allocator* const allocator;
  std::mutex mutex;
  Stats stats_;
};

size_t TrackedAllocator::Stats::numAllocations() const {
  size_t out = 0;
  for (auto& stats : byUsage) {
    out += stats.count;
  }
  return out;
}

size_t TrackedAllocator::Stats::bytesAllocated() const {
  size_t out = 0;
  for (auto& stats : byUsage) {
    out += stats.bytes;
  }
  return out;
}

TrackedAllocator::TrackedAllocator(Allocator* allocator_)
    : allocator(allocator_) {}

TrackedAllocator::Stats TrackedAllocator::stats() {
  std::unique_lock<std::mutex> lock(mutex);
  return stats_;
}

Allocation TrackedAllocator::allocate(const Allocation::Request& request) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    auto& usageStats = stats_.byUsage[int(request.usage)];
    ++usageStats.count;
    usageStats.bytes += request.size;
  }
  return allocator->allocate(request);
}

void TrackedAllocator::free(const Allocation& allocation) {
  {
    std::unique_lock<std::mutex> lock(mutex);
    auto& usageStats = stats_.byUsage[int(allocation.request.usage)];
    MARL_ASSERT(usageStats.count > 0,
                "TrackedAllocator detected abnormal free()");
    MARL_ASSERT(usageStats.bytes >= allocation.request.size,
                "TrackedAllocator detected abnormal free()");
    --usageStats.count;
    usageStats.bytes -= allocation.request.size;
  }
  return allocator->free(allocation);
}

///////////////////////////////////////////////////////////////////////////////
// StlAllocator
///////////////////////////////////////////////////////////////////////////////

// StlAllocator exposes an STL-compatible allocator wrapping a marl::Allocator.
template <typename T>
struct StlAllocator {
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = size_t;
  using difference_type = size_t;

  // An equivalent STL allocator for a different type.
  template <class U>
  struct rebind {
    typedef StlAllocator<U> other;
  };

  // Constructs an StlAllocator that will allocate using allocator.
  // allocator must remain valid until this StlAllocator has been destroyed.
  inline StlAllocator(Allocator* allocator);

  template <typename U>
  inline StlAllocator(const StlAllocator<U>& other);

  // Returns the actual address of x even in presence of overloaded operator&.
  inline pointer address(reference x) const;
  inline const_pointer address(const_reference x) const;

  // Allocates the memory for n objects of type T.
  // Does not actually construct the objects.
  inline T* allocate(std::size_t n);

  // Deallocates the memory for n objects of type T.
  inline void deallocate(T* p, std::size_t n);

  // Returns the maximum theoretically possible number of T stored in this
  // allocator.
  inline size_type max_size() const;

  // Copy constructs an object of type T at the address p.
  inline void construct(pointer p, const_reference val);

  // Constructs an object of type U at the address P forwarning all other
  // arguments to the constructor.
  template <typename U, typename... Args>
  inline void construct(U* p, Args&&... args);

  // Deconstructs the object at p. It does not free the memory.
  inline void destroy(pointer p);

  // Deconstructs the object at p. It does not free the memory.
  template <typename U>
  inline void destroy(U* p);

 private:
  inline Allocation::Request request(size_t n) const;

  template <typename U>
  friend struct StlAllocator;
  Allocator* allocator;
};

template <typename T>
StlAllocator<T>::StlAllocator(Allocator* allocator_) : allocator(allocator_) {}

template <typename T>
template <typename U>
StlAllocator<T>::StlAllocator(const StlAllocator<U>& other) {
  allocator = other.allocator;
}

template <typename T>
typename StlAllocator<T>::pointer StlAllocator<T>::address(reference x) const {
  return &x;
}
template <typename T>
typename StlAllocator<T>::const_pointer StlAllocator<T>::address(
    const_reference x) const {
  return &x;
}

template <typename T>
T* StlAllocator<T>::allocate(std::size_t n) {
  auto alloc = allocator->allocate(request(n));
  return reinterpret_cast<T*>(alloc.ptr);
}

template <typename T>
void StlAllocator<T>::deallocate(T* p, std::size_t n) {
  Allocation alloc;
  alloc.ptr = p;
  alloc.request = request(n);
  allocator->free(alloc);
}

template <typename T>
typename StlAllocator<T>::size_type StlAllocator<T>::max_size() const {
  return std::numeric_limits<size_type>::max() / sizeof(value_type);
}

template <typename T>
void StlAllocator<T>::construct(pointer p, const_reference val) {
  new (p) T(val);
}

template <typename T>
template <typename U, typename... Args>
void StlAllocator<T>::construct(U* p, Args&&... args) {
  ::new ((void*)p) U(std::forward<Args>(args)...);
}

template <typename T>
void StlAllocator<T>::destroy(pointer p) {
  ((T*)p)->~T();
}

template <typename T>
template <typename U>
void StlAllocator<T>::destroy(U* p) {
  p->~U();
}

template <typename T>
Allocation::Request StlAllocator<T>::request(size_t n) const {
  Allocation::Request req = {};
  req.size = sizeof(T) * n;
  req.alignment = alignof(T);
  req.usage = Allocation::Usage::Stl;
  return req;
}

}  // namespace marl

#endif  // marl_memory_h
