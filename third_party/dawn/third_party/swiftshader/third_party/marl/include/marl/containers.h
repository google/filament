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

#ifndef marl_containers_h
#define marl_containers_h

#include "debug.h"
#include "memory.h"

#include <algorithm>  // std::max
#include <cstddef>    // size_t
#include <utility>    // std::move

#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace marl {
namespace containers {

////////////////////////////////////////////////////////////////////////////////
// STL wrappers
// STL containers that use a marl::StlAllocator backed by a marl::Allocator.
// Note: These may be re-implemented to optimize for marl's usage cases.
// See: https://github.com/google/marl/issues/129
////////////////////////////////////////////////////////////////////////////////
template <typename T>
using deque = std::deque<T, StlAllocator<T>>;

template <typename K, typename V, typename C = std::less<K>>
using map = std::map<K, V, C, StlAllocator<std::pair<const K, V>>>;

template <typename K, typename C = std::less<K>>
using set = std::set<K, C, StlAllocator<K>>;

template <typename K,
          typename V,
          typename H = std::hash<K>,
          typename E = std::equal_to<K>>
using unordered_map =
    std::unordered_map<K, V, H, E, StlAllocator<std::pair<const K, V>>>;

template <typename K, typename H = std::hash<K>, typename E = std::equal_to<K>>
using unordered_set = std::unordered_set<K, H, E, StlAllocator<K>>;

// take() takes and returns the front value from the deque.
template <typename T>
MARL_NO_EXPORT inline T take(deque<T>& queue) {
  auto out = std::move(queue.front());
  queue.pop_front();
  return out;
}

// take() takes and returns the first value from the unordered_set.
template <typename T, typename H, typename E>
MARL_NO_EXPORT inline T take(unordered_set<T, H, E>& set) {
  auto it = set.begin();
  auto out = std::move(*it);
  set.erase(it);
  return out;
}

////////////////////////////////////////////////////////////////////////////////
// vector<T, BASE_CAPACITY>
////////////////////////////////////////////////////////////////////////////////

// vector is a container of contiguously stored elements.
// Unlike std::vector, marl::containers::vector keeps the first
// BASE_CAPACITY elements internally, which will avoid dynamic heap
// allocations. Once the vector exceeds BASE_CAPACITY elements, vector will
// allocate storage from the heap.
template <typename T, int BASE_CAPACITY>
class vector {
 public:
  MARL_NO_EXPORT inline vector(Allocator* allocator = Allocator::Default);

  template <int BASE_CAPACITY_2>
  MARL_NO_EXPORT inline vector(const vector<T, BASE_CAPACITY_2>& other,
                               Allocator* allocator = Allocator::Default);

  template <int BASE_CAPACITY_2>
  MARL_NO_EXPORT inline vector(vector<T, BASE_CAPACITY_2>&& other,
                               Allocator* allocator = Allocator::Default);

  MARL_NO_EXPORT inline ~vector();

  MARL_NO_EXPORT inline vector& operator=(const vector&);

  template <int BASE_CAPACITY_2>
  MARL_NO_EXPORT inline vector<T, BASE_CAPACITY>& operator=(
      const vector<T, BASE_CAPACITY_2>&);

  template <int BASE_CAPACITY_2>
  MARL_NO_EXPORT inline vector<T, BASE_CAPACITY>& operator=(
      vector<T, BASE_CAPACITY_2>&&);

  MARL_NO_EXPORT inline void push_back(const T& el);
  MARL_NO_EXPORT inline void emplace_back(T&& el);
  MARL_NO_EXPORT inline void pop_back();
  MARL_NO_EXPORT inline T& front();
  MARL_NO_EXPORT inline T& back();
  MARL_NO_EXPORT inline const T& front() const;
  MARL_NO_EXPORT inline const T& back() const;
  MARL_NO_EXPORT inline T* begin();
  MARL_NO_EXPORT inline T* end();
  MARL_NO_EXPORT inline const T* begin() const;
  MARL_NO_EXPORT inline const T* end() const;
  MARL_NO_EXPORT inline T& operator[](size_t i);
  MARL_NO_EXPORT inline const T& operator[](size_t i) const;
  MARL_NO_EXPORT inline size_t size() const;
  MARL_NO_EXPORT inline size_t cap() const;
  MARL_NO_EXPORT inline void resize(size_t n);
  MARL_NO_EXPORT inline void reserve(size_t n);
  MARL_NO_EXPORT inline T* data();
  MARL_NO_EXPORT inline const T* data() const;

  Allocator* const allocator;

 private:
  using TStorage = typename marl::aligned_storage<sizeof(T), alignof(T)>::type;

  vector(const vector&) = delete;

  MARL_NO_EXPORT inline void free();

  size_t count = 0;
  size_t capacity = BASE_CAPACITY;
  TStorage buffer[BASE_CAPACITY];
  TStorage* elements = buffer;
  Allocation allocation;
};

template <typename T, int BASE_CAPACITY>
vector<T, BASE_CAPACITY>::vector(
    Allocator* allocator_ /* = Allocator::Default */)
    : allocator(allocator_) {}

template <typename T, int BASE_CAPACITY>
template <int BASE_CAPACITY_2>
vector<T, BASE_CAPACITY>::vector(
    const vector<T, BASE_CAPACITY_2>& other,
    Allocator* allocator_ /* = Allocator::Default */)
    : allocator(allocator_) {
  *this = other;
}

template <typename T, int BASE_CAPACITY>
template <int BASE_CAPACITY_2>
vector<T, BASE_CAPACITY>::vector(
    vector<T, BASE_CAPACITY_2>&& other,
    Allocator* allocator_ /* = Allocator::Default */)
    : allocator(allocator_) {
  *this = std::move(other);
}

template <typename T, int BASE_CAPACITY>
vector<T, BASE_CAPACITY>::~vector() {
  free();
}

template <typename T, int BASE_CAPACITY>
vector<T, BASE_CAPACITY>& vector<T, BASE_CAPACITY>::operator=(
    const vector& other) {
  free();
  reserve(other.size());
  count = other.size();
  for (size_t i = 0; i < count; i++) {
    new (&reinterpret_cast<T*>(elements)[i]) T(other[i]);
  }
  return *this;
}

template <typename T, int BASE_CAPACITY>
template <int BASE_CAPACITY_2>
vector<T, BASE_CAPACITY>& vector<T, BASE_CAPACITY>::operator=(
    const vector<T, BASE_CAPACITY_2>& other) {
  free();
  reserve(other.size());
  count = other.size();
  for (size_t i = 0; i < count; i++) {
    new (&reinterpret_cast<T*>(elements)[i]) T(other[i]);
  }
  return *this;
}

template <typename T, int BASE_CAPACITY>
template <int BASE_CAPACITY_2>
vector<T, BASE_CAPACITY>& vector<T, BASE_CAPACITY>::operator=(
    vector<T, BASE_CAPACITY_2>&& other) {
  free();
  reserve(other.size());
  count = other.size();
  for (size_t i = 0; i < count; i++) {
    new (&reinterpret_cast<T*>(elements)[i]) T(std::move(other[i]));
  }
  other.resize(0);
  return *this;
}

template <typename T, int BASE_CAPACITY>
void vector<T, BASE_CAPACITY>::push_back(const T& el) {
  reserve(count + 1);
  new (&reinterpret_cast<T*>(elements)[count]) T(el);
  count++;
}

template <typename T, int BASE_CAPACITY>
void vector<T, BASE_CAPACITY>::emplace_back(T&& el) {
  reserve(count + 1);
  new (&reinterpret_cast<T*>(elements)[count]) T(std::move(el));
  count++;
}

template <typename T, int BASE_CAPACITY>
void vector<T, BASE_CAPACITY>::pop_back() {
  MARL_ASSERT(count > 0, "pop_back() called on empty vector");
  count--;
  reinterpret_cast<T*>(elements)[count].~T();
}

template <typename T, int BASE_CAPACITY>
T& vector<T, BASE_CAPACITY>::front() {
  MARL_ASSERT(count > 0, "front() called on empty vector");
  return reinterpret_cast<T*>(elements)[0];
}

template <typename T, int BASE_CAPACITY>
T& vector<T, BASE_CAPACITY>::back() {
  MARL_ASSERT(count > 0, "back() called on empty vector");
  return reinterpret_cast<T*>(elements)[count - 1];
}

template <typename T, int BASE_CAPACITY>
const T& vector<T, BASE_CAPACITY>::front() const {
  MARL_ASSERT(count > 0, "front() called on empty vector");
  return reinterpret_cast<T*>(elements)[0];
}

template <typename T, int BASE_CAPACITY>
const T& vector<T, BASE_CAPACITY>::back() const {
  MARL_ASSERT(count > 0, "back() called on empty vector");
  return reinterpret_cast<T*>(elements)[count - 1];
}

template <typename T, int BASE_CAPACITY>
T* vector<T, BASE_CAPACITY>::begin() {
  return reinterpret_cast<T*>(elements);
}

template <typename T, int BASE_CAPACITY>
T* vector<T, BASE_CAPACITY>::end() {
  return reinterpret_cast<T*>(elements) + count;
}

template <typename T, int BASE_CAPACITY>
const T* vector<T, BASE_CAPACITY>::begin() const {
  return reinterpret_cast<T*>(elements);
}

template <typename T, int BASE_CAPACITY>
const T* vector<T, BASE_CAPACITY>::end() const {
  return reinterpret_cast<T*>(elements) + count;
}

template <typename T, int BASE_CAPACITY>
T& vector<T, BASE_CAPACITY>::operator[](size_t i) {
  MARL_ASSERT(i < count, "index %d exceeds vector size %d", int(i), int(count));
  return reinterpret_cast<T*>(elements)[i];
}

template <typename T, int BASE_CAPACITY>
const T& vector<T, BASE_CAPACITY>::operator[](size_t i) const {
  MARL_ASSERT(i < count, "index %d exceeds vector size %d", int(i), int(count));
  return reinterpret_cast<T*>(elements)[i];
}

template <typename T, int BASE_CAPACITY>
size_t vector<T, BASE_CAPACITY>::size() const {
  return count;
}

template <typename T, int BASE_CAPACITY>
void vector<T, BASE_CAPACITY>::resize(size_t n) {
  reserve(n);
  while (count < n) {
    new (&reinterpret_cast<T*>(elements)[count++]) T();
  }
  while (n < count) {
    reinterpret_cast<T*>(elements)[--count].~T();
  }
}

template <typename T, int BASE_CAPACITY>
void vector<T, BASE_CAPACITY>::reserve(size_t n) {
  if (n > capacity) {
    capacity = std::max<size_t>(n * 2, 8);

    Allocation::Request request;
    request.size = sizeof(T) * capacity;
    request.alignment = alignof(T);
    request.usage = Allocation::Usage::Vector;

    auto alloc = allocator->allocate(request);
    auto grown = reinterpret_cast<TStorage*>(alloc.ptr);
    for (size_t i = 0; i < count; i++) {
      new (&reinterpret_cast<T*>(grown)[i])
          T(std::move(reinterpret_cast<T*>(elements)[i]));
    }
    free();
    elements = grown;
    allocation = alloc;
  }
}

template <typename T, int BASE_CAPACITY>
T* vector<T, BASE_CAPACITY>::data() {
  return elements;
}

template <typename T, int BASE_CAPACITY>
const T* vector<T, BASE_CAPACITY>::data() const {
  return elements;
}

template <typename T, int BASE_CAPACITY>
void vector<T, BASE_CAPACITY>::free() {
  for (size_t i = 0; i < count; i++) {
    reinterpret_cast<T*>(elements)[i].~T();
  }

  if (allocation.ptr != nullptr) {
    allocator->free(allocation);
    allocation = {};
    elements = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////
// list<T, BASE_CAPACITY>
////////////////////////////////////////////////////////////////////////////////

// list is a minimal std::list like container that supports constant time
// insertion and removal of elements.
// list keeps hold of allocations (it only releases allocations on destruction),
// to avoid repeated heap allocations and frees when frequently inserting and
// removing elements.
template <typename T>
class list {
  struct Entry {
    T data;
    Entry* next;
    Entry* prev;
  };

 public:
  class iterator {
   public:
    MARL_NO_EXPORT inline iterator(Entry*);
    MARL_NO_EXPORT inline T* operator->();
    MARL_NO_EXPORT inline T& operator*();
    MARL_NO_EXPORT inline iterator& operator++();
    MARL_NO_EXPORT inline bool operator==(const iterator&) const;
    MARL_NO_EXPORT inline bool operator!=(const iterator&) const;

   private:
    friend list;
    Entry* entry;
  };

  MARL_NO_EXPORT inline list(Allocator* allocator = Allocator::Default);
  MARL_NO_EXPORT inline ~list();

  MARL_NO_EXPORT inline iterator begin();
  MARL_NO_EXPORT inline iterator end();
  MARL_NO_EXPORT inline size_t size() const;

  template <typename... Args>
  MARL_NO_EXPORT inline iterator emplace_front(Args&&... args);
  MARL_NO_EXPORT inline void erase(iterator);

 private:
  // copy / move is currently unsupported.
  list(const list&) = delete;
  list(list&&) = delete;
  list& operator=(const list&) = delete;
  list& operator=(list&&) = delete;

  struct AllocationChain {
    Allocation allocation;
    AllocationChain* next;
  };

  MARL_NO_EXPORT inline void grow(size_t count);

  MARL_NO_EXPORT static inline void unlink(Entry* entry, Entry*& list);
  MARL_NO_EXPORT static inline void link(Entry* entry, Entry*& list);

  Allocator* const allocator;
  size_t size_ = 0;
  size_t capacity = 0;
  AllocationChain* allocations = nullptr;
  Entry* free = nullptr;
  Entry* head = nullptr;
};

template <typename T>
list<T>::iterator::iterator(Entry* entry_) : entry(entry_) {}

template <typename T>
T* list<T>::iterator::operator->() {
  return &entry->data;
}

template <typename T>
T& list<T>::iterator::operator*() {
  return entry->data;
}

template <typename T>
typename list<T>::iterator& list<T>::iterator::operator++() {
  entry = entry->next;
  return *this;
}

template <typename T>
bool list<T>::iterator::operator==(const iterator& rhs) const {
  return entry == rhs.entry;
}

template <typename T>
bool list<T>::iterator::operator!=(const iterator& rhs) const {
  return entry != rhs.entry;
}

template <typename T>
list<T>::list(Allocator* allocator_ /* = Allocator::Default */)
    : allocator(allocator_) {}

template <typename T>
list<T>::~list() {
  for (auto el = head; el != nullptr; el = el->next) {
    el->data.~T();
  }

  auto curr = allocations;
  while (curr != nullptr) {
    auto next = curr->next;
    allocator->free(curr->allocation);
    curr = next;
  }
}

template <typename T>
typename list<T>::iterator list<T>::begin() {
  return {head};
}

template <typename T>
typename list<T>::iterator list<T>::end() {
  return {nullptr};
}

template <typename T>
size_t list<T>::size() const {
  return size_;
}

template <typename T>
template <typename... Args>
typename list<T>::iterator list<T>::emplace_front(Args&&... args) {
  if (free == nullptr) {
    grow(std::max<size_t>(capacity, 8));
  }

  auto entry = free;

  unlink(entry, free);
  link(entry, head);

  new (&entry->data) T(std::forward<T>(args)...);
  size_++;

  return entry;
}

template <typename T>
void list<T>::erase(iterator it) {
  auto entry = it.entry;
  unlink(entry, head);
  link(entry, free);

  entry->data.~T();
  size_--;
}

template <typename T>
void list<T>::grow(size_t count) {
  auto const entriesSize = sizeof(Entry) * count;
  auto const allocChainOffset = alignUp(entriesSize, alignof(AllocationChain));
  auto const allocSize = allocChainOffset + sizeof(AllocationChain);

  Allocation::Request request;
  request.size = allocSize;
  request.alignment = std::max(alignof(Entry), alignof(AllocationChain));
  request.usage = Allocation::Usage::List;
  auto alloc = allocator->allocate(request);

  auto entries = reinterpret_cast<Entry*>(alloc.ptr);
  for (size_t i = 0; i < count; i++) {
    auto entry = &entries[i];
    entry->prev = nullptr;
    entry->next = free;
    if (free) {
      free->prev = entry;
    }
    free = entry;
  }

  auto allocChain = reinterpret_cast<AllocationChain*>(
      reinterpret_cast<uint8_t*>(alloc.ptr) + allocChainOffset);

  allocChain->allocation = alloc;
  allocChain->next = allocations;
  allocations = allocChain;

  capacity += count;
}

template <typename T>
void list<T>::unlink(Entry* entry, Entry*& list) {
  if (list == entry) {
    list = list->next;
  }
  if (entry->prev) {
    entry->prev->next = entry->next;
  }
  if (entry->next) {
    entry->next->prev = entry->prev;
  }
  entry->prev = nullptr;
  entry->next = nullptr;
}

template <typename T>
void list<T>::link(Entry* entry, Entry*& list) {
  MARL_ASSERT(entry->next == nullptr, "link() called on entry already linked");
  MARL_ASSERT(entry->prev == nullptr, "link() called on entry already linked");
  if (list) {
    entry->next = list;
    list->prev = entry;
  }
  list = entry;
}

}  // namespace containers
}  // namespace marl

#endif  // marl_containers_h
