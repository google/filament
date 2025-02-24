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

#ifndef marl_pool_h
#define marl_pool_h

#include "conditionvariable.h"
#include "memory.h"
#include "mutex.h"

#include <atomic>

namespace marl {

// PoolPolicy controls whether pool items are constructed and destructed each
// time they are borrowed from and returned to a pool, or whether they persist
// constructed for the lifetime of the pool.
enum class PoolPolicy {
  // Call the Pool items constructor on borrow(), and destruct the item
  // when the item is returned.
  Reconstruct,

  // Construct and destruct all items once for the lifetime of the Pool.
  // Items will keep their state between loans.
  Preserve,
};

////////////////////////////////////////////////////////////////////////////////
// Pool<T>
////////////////////////////////////////////////////////////////////////////////

// Pool is the abstract base class for BoundedPool<> and UnboundedPool<>.
template <typename T>
class Pool {
 protected:
  struct Item;
  class Storage;

 public:
  // A Loan is returned by the pool's borrow() function.
  // Loans track the number of references to the loaned item, and return the
  // item to the pool when the final Loan reference is dropped.
  class Loan {
   public:
    MARL_NO_EXPORT inline Loan() = default;
    MARL_NO_EXPORT inline Loan(Item*, const std::shared_ptr<Storage>&);
    MARL_NO_EXPORT inline Loan(const Loan&);
    MARL_NO_EXPORT inline Loan(Loan&&);
    MARL_NO_EXPORT inline ~Loan();
    MARL_NO_EXPORT inline Loan& operator=(const Loan&);
    MARL_NO_EXPORT inline Loan& operator=(Loan&&);
    MARL_NO_EXPORT inline T& operator*();
    MARL_NO_EXPORT inline T* operator->() const;
    MARL_NO_EXPORT inline T* get() const;
    MARL_NO_EXPORT inline void reset();

   private:
    Item* item = nullptr;
    std::shared_ptr<Storage> storage;
  };

 protected:
  Pool() = default;

  // The shared storage between the pool and all loans.
  class Storage {
   public:
    virtual ~Storage() = default;
    virtual void return_(Item*) = 0;
  };

  // The backing data of a single item in the pool.
  struct Item {
    // get() returns a pointer to the item's data.
    MARL_NO_EXPORT inline T* get();

    // construct() calls the constructor on the item's data.
    MARL_NO_EXPORT inline void construct();

    // destruct() calls the destructor on the item's data.
    MARL_NO_EXPORT inline void destruct();

    using Data = typename aligned_storage<sizeof(T), alignof(T)>::type;
    Data data;
    std::atomic<int> refcount = {0};
    Item* next = nullptr;  // pointer to the next free item in the pool.
  };
};

// Loan<T> is an alias to Pool<T>::Loan.
template <typename T>
using Loan = typename Pool<T>::Loan;

////////////////////////////////////////////////////////////////////////////////
// Pool<T>::Item
////////////////////////////////////////////////////////////////////////////////
template <typename T>
T* Pool<T>::Item::get() {
  return reinterpret_cast<T*>(&data);
}

template <typename T>
void Pool<T>::Item::construct() {
  new (&data) T;
}

template <typename T>
void Pool<T>::Item::destruct() {
  get()->~T();
}

////////////////////////////////////////////////////////////////////////////////
// Pool<T>::Loan
////////////////////////////////////////////////////////////////////////////////
template <typename T>
Pool<T>::Loan::Loan(Item* item, const std::shared_ptr<Storage>& storage)
    : item(item), storage(storage) {
  item->refcount++;
}

template <typename T>
Pool<T>::Loan::Loan(const Loan& other)
    : item(other.item), storage(other.storage) {
  if (item != nullptr) {
    item->refcount++;
  }
}

template <typename T>
Pool<T>::Loan::Loan(Loan&& other) : item(other.item), storage(other.storage) {
  other.item = nullptr;
  other.storage = nullptr;
}

template <typename T>
Pool<T>::Loan::~Loan() {
  reset();
}

template <typename T>
void Pool<T>::Loan::reset() {
  if (item != nullptr) {
    auto refs = --item->refcount;
    MARL_ASSERT(refs >= 0, "reset() called on zero-ref pool item");
    if (refs == 0) {
      storage->return_(item);
    }
    item = nullptr;
    storage = nullptr;
  }
}

template <typename T>
typename Pool<T>::Loan& Pool<T>::Loan::operator=(const Loan& rhs) {
  reset();
  if (rhs.item != nullptr) {
    item = rhs.item;
    storage = rhs.storage;
    rhs.item->refcount++;
  }
  return *this;
}

template <typename T>
typename Pool<T>::Loan& Pool<T>::Loan::operator=(Loan&& rhs) {
  reset();
  std::swap(item, rhs.item);
  std::swap(storage, rhs.storage);
  return *this;
}

template <typename T>
T& Pool<T>::Loan::operator*() {
  return *item->get();
}

template <typename T>
T* Pool<T>::Loan::operator->() const {
  return item->get();
}

template <typename T>
T* Pool<T>::Loan::get() const {
  return item ? item->get() : nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// BoundedPool<T, N, POLICY>
////////////////////////////////////////////////////////////////////////////////

// BoundedPool<T, N, POLICY> is a pool of items of type T, with a maximum
// capacity of N items.
// BoundedPool<> is initially populated with N default-constructed items.
// POLICY controls whether pool items are constructed and destructed each
// time they are borrowed from and returned to the pool.
template <typename T, int N, PoolPolicy POLICY = PoolPolicy::Reconstruct>
class BoundedPool : public Pool<T> {
 public:
  using Item = typename Pool<T>::Item;
  using Loan = typename Pool<T>::Loan;

  MARL_NO_EXPORT inline BoundedPool(Allocator* allocator = Allocator::Default);

  // borrow() borrows a single item from the pool, blocking until an item is
  // returned if the pool is empty.
  MARL_NO_EXPORT inline Loan borrow() const;

  // borrow() borrows count items from the pool, blocking until there are at
  // least count items in the pool. The function f() is called with each
  // borrowed item.
  // F must be a function with the signature: void(T&&)
  template <typename F>
  MARL_NO_EXPORT inline void borrow(size_t count, const F& f) const;

  // tryBorrow() attempts to borrow a single item from the pool without
  // blocking.
  // The boolean of the returned pair is true on success, or false if the pool
  // is empty.
  MARL_NO_EXPORT inline std::pair<Loan, bool> tryBorrow() const;

 private:
  class Storage : public Pool<T>::Storage {
   public:
    MARL_NO_EXPORT inline Storage(Allocator* allocator);
    MARL_NO_EXPORT inline ~Storage();
    MARL_NO_EXPORT inline void return_(Item*) override;
    // We cannot copy this as the Item pointers would be shared and
    // deleted at a wrong point. We cannot move this because we return
    // pointers into items[N].
    MARL_NO_EXPORT inline Storage(const Storage&) = delete;
    MARL_NO_EXPORT inline Storage& operator=(const Storage&) = delete;

    Item items[N];
    marl::mutex mutex;
    ConditionVariable returned;
    Item* free = nullptr;
  };
  std::shared_ptr<Storage> storage;
};

template <typename T, int N, PoolPolicy POLICY>
BoundedPool<T, N, POLICY>::Storage::Storage(Allocator* allocator)
    : returned(allocator) {
  for (int i = 0; i < N; i++) {
    if (POLICY == PoolPolicy::Preserve) {
      items[i].construct();
    }
    items[i].next = this->free;
    this->free = &items[i];
  }
}

template <typename T, int N, PoolPolicy POLICY>
BoundedPool<T, N, POLICY>::Storage::~Storage() {
  if (POLICY == PoolPolicy::Preserve) {
    for (int i = 0; i < N; i++) {
      items[i].destruct();
    }
  }
}

template <typename T, int N, PoolPolicy POLICY>
BoundedPool<T, N, POLICY>::BoundedPool(
    Allocator* allocator /* = Allocator::Default */)
    : storage(allocator->make_shared<Storage>(allocator)) {}

template <typename T, int N, PoolPolicy POLICY>
typename BoundedPool<T, N, POLICY>::Loan BoundedPool<T, N, POLICY>::borrow()
    const {
  Loan out;
  borrow(1, [&](Loan&& loan) { out = std::move(loan); });
  return out;
}

template <typename T, int N, PoolPolicy POLICY>
template <typename F>
void BoundedPool<T, N, POLICY>::borrow(size_t n, const F& f) const {
  marl::lock lock(storage->mutex);
  for (size_t i = 0; i < n; i++) {
    storage->returned.wait(lock, [&] { return storage->free != nullptr; });
    auto item = storage->free;
    storage->free = storage->free->next;
    if (POLICY == PoolPolicy::Reconstruct) {
      item->construct();
    }
    f(std::move(Loan(item, storage)));
  }
}

template <typename T, int N, PoolPolicy POLICY>
std::pair<typename BoundedPool<T, N, POLICY>::Loan, bool>
BoundedPool<T, N, POLICY>::tryBorrow() const {
  Item* item = nullptr;
  {
    marl::lock lock(storage->mutex);
    if (storage->free == nullptr) {
      return std::make_pair(Loan(), false);
    }
    item = storage->free;
    storage->free = storage->free->next;
    item->pool = this;
  }
  if (POLICY == PoolPolicy::Reconstruct) {
    item->construct();
  }
  return std::make_pair(Loan(item, storage), true);
}

template <typename T, int N, PoolPolicy POLICY>
void BoundedPool<T, N, POLICY>::Storage::return_(Item* item) {
  if (POLICY == PoolPolicy::Reconstruct) {
    item->destruct();
  }
  {
    marl::lock lock(mutex);
    item->next = free;
    free = item;
  }
  returned.notify_one();
}

////////////////////////////////////////////////////////////////////////////////
// UnboundedPool
////////////////////////////////////////////////////////////////////////////////

// UnboundedPool<T, POLICY> is a pool of items of type T.
// UnboundedPool<> will automatically allocate more items if the pool becomes
// empty.
// POLICY controls whether pool items are constructed and destructed each
// time they are borrowed from and returned to the pool.
template <typename T, PoolPolicy POLICY = PoolPolicy::Reconstruct>
class UnboundedPool : public Pool<T> {
 public:
  using Item = typename Pool<T>::Item;
  using Loan = typename Pool<T>::Loan;

  MARL_NO_EXPORT inline UnboundedPool(
      Allocator* allocator = Allocator::Default);

  // borrow() borrows a single item from the pool, automatically allocating
  // more items if the pool is empty.
  // This function does not block.
  MARL_NO_EXPORT inline Loan borrow() const;

  // borrow() borrows count items from the pool, calling the function f() with
  // each borrowed item.
  // F must be a function with the signature: void(T&&)
  // This function does not block.
  template <typename F>
  MARL_NO_EXPORT inline void borrow(size_t n, const F& f) const;

 private:
  class Storage : public Pool<T>::Storage {
   public:
    MARL_NO_EXPORT inline Storage(Allocator* allocator);
    MARL_NO_EXPORT inline ~Storage();
    MARL_NO_EXPORT inline void return_(Item*) override;
    // We cannot copy this as the Item pointers would be shared and
    // deleted at a wrong point. We could move this but would have to take
    // extra care no Item pointers are left in the moved-out object.
    MARL_NO_EXPORT inline Storage(const Storage&) = delete;
    MARL_NO_EXPORT inline Storage& operator=(const Storage&) = delete;

    Allocator* allocator;
    marl::mutex mutex;
    containers::vector<Item*, 4> items;
    Item* free = nullptr;
  };

  Allocator* allocator;
  std::shared_ptr<Storage> storage;
};

template <typename T, PoolPolicy POLICY>
UnboundedPool<T, POLICY>::Storage::Storage(Allocator* allocator)
    : allocator(allocator), items(allocator) {}

template <typename T, PoolPolicy POLICY>
UnboundedPool<T, POLICY>::Storage::~Storage() {
  for (auto item : items) {
    if (POLICY == PoolPolicy::Preserve) {
      item->destruct();
    }
    allocator->destroy(item);
  }
}

template <typename T, PoolPolicy POLICY>
UnboundedPool<T, POLICY>::UnboundedPool(
    Allocator* allocator /* = Allocator::Default */)
    : allocator(allocator),
      storage(allocator->make_shared<Storage>(allocator)) {}

template <typename T, PoolPolicy POLICY>
Loan<T> UnboundedPool<T, POLICY>::borrow() const {
  Loan out;
  borrow(1, [&](Loan&& loan) { out = std::move(loan); });
  return out;
}

template <typename T, PoolPolicy POLICY>
template <typename F>
inline void UnboundedPool<T, POLICY>::borrow(size_t n, const F& f) const {
  marl::lock lock(storage->mutex);
  for (size_t i = 0; i < n; i++) {
    if (storage->free == nullptr) {
      auto count = std::max<size_t>(storage->items.size(), 32);
      for (size_t j = 0; j < count; j++) {
        auto item = allocator->create<Item>();
        if (POLICY == PoolPolicy::Preserve) {
          item->construct();
        }
        storage->items.push_back(item);
        item->next = storage->free;
        storage->free = item;
      }
    }

    auto item = storage->free;
    storage->free = storage->free->next;
    if (POLICY == PoolPolicy::Reconstruct) {
      item->construct();
    }
    f(std::move(Loan(item, storage)));
  }
}

template <typename T, PoolPolicy POLICY>
void UnboundedPool<T, POLICY>::Storage::return_(Item* item) {
  if (POLICY == PoolPolicy::Reconstruct) {
    item->destruct();
  }
  marl::lock lock(mutex);
  item->next = free;
  free = item;
}

}  // namespace marl

#endif  // marl_pool_h
