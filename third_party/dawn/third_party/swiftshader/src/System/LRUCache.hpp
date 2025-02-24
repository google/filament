// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef sw_LRUCache_hpp
#define sw_LRUCache_hpp

#include "System/Debug.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_set>
#include <vector>

namespace sw {

// LRUCache is a least recently used cache of a fixed capacity.
template<typename KEY, typename DATA, typename HASH = std::hash<KEY> >
class LRUCache
{
	struct Entry;

public:
	using Key = KEY;
	using Data = DATA;
	using Hash = HASH;

	// view is a const accessor of a single cache entry.
	class view
	{
	public:
		inline view(Entry *);

		inline const Key &key() const;
		inline const Data &data() const;

	private:
		Entry *entry;
	};

	class iterator
	{
	public:
		inline iterator(Entry *);
		inline view operator*() const;
		inline iterator &operator++();
		inline bool operator==(const iterator &) const;
		inline bool operator!=(const iterator &) const;

	private:
		Entry *entry;
	};

	// Construct a LRU cache with the given maximum number of entries.
	inline LRUCache(size_t capacity);
	inline ~LRUCache() = default;

	// lookup() looks up the cache entry with the given key.
	// If the entry is found, this is moved to the most-recent position in the
	// cache, and its data is returned.
	// If the entry is not found, then a default initialized Data is returned.
	inline Data lookup(const Key &key);

	// add() adds the data to the cache using the given key, placed at the
	// most-recent position in the cache.
	// If an existing entry exists in the cache with the given key, then this is
	// replaced with data.
	// If no existing entry exists in the cache, and the cache is already full
	// then the least recently used entry is evicted before adding the new
	// entry.
	inline void add(const Key &key, const Data &data);

	// clear() clears the cache of all elements.
	inline void clear();

	// Range based iterators.
	inline iterator begin() const;
	inline iterator end() const;

private:
	LRUCache(const LRUCache &) = delete;
	LRUCache(LRUCache &&) = delete;
	LRUCache &operator=(const LRUCache &) = delete;
	LRUCache &operator=(LRUCache &&) = delete;

	// Keyed holds a key. See find() for more information.
	struct Keyed
	{
		Key key = {};
	};

	// Cache entry structure.
	// Holds the unique copy of the key and data, along with pointers for
	// maintaining the linked list.
	struct Entry : Keyed
	{
		Data data = {};
		Entry *next = nullptr;
		Entry *prev = nullptr;
	};

	// KeyedComparator is a custom hasher and equality helper for Keyed.
	struct KeyedComparator
	{
		// Hash function.
		inline uint64_t operator()(const Keyed *k) const;

		// Equality function.
		inline uint64_t operator()(const Keyed *a, const Keyed *b) const;
	};

	// find() returns the Entry* for the given key, or nullptr.
	// find() performs this by casting the Key pointer to a Keyed pointer for
	// searching the std::unordered_set.
	//
	// This is to avoid having a duplicate Key held by both an
	// unordered_map<Key, Entry*> and the Entry itself, as the Key type may be
	// large.
	//
	// While we could use an unordered_set<Entry*>, this then requires the
	// construction of a temporary Entry to perform the call to
	// unordered_set<Entry*>::find(). This is undesirable as the Data type might
	// be large or have an expensive constructor.
	//
	// C++20 gains a new templated overload for unordered_set<Entry*>::find()
	// which would allow us to use unordered_set<Entry*>::find(Key*).
	// Until we can use C++20, keep this casting nastiness hidden away in this
	// one function.
	inline Entry *find(const Key &key);

	// unlinks the entry from the list it is linked in.
	inline void unlink(Entry *);

	// links the entry to the head of the LRU.
	inline void link(Entry *);

	// storage holds the allocations of all the entries.
	// This vector must not change size for the lifetime of the cache.
	std::vector<Entry> storage;

	// set is an unordered set of Keyed*, using the KeyedComparator for hash and
	// equality testing.
	std::unordered_set<const Keyed *, KeyedComparator, KeyedComparator> set;

	Entry *free = nullptr;  // Singly-linked (prev is nullptr) list of free entries.
	Entry *head = nullptr;  // Pointer to the most recently used entry in the LRU.
	Entry *tail = nullptr;  // Pointer to the least recently used entry in the LRU.
};

////////////////////////////////////////////////////////////////////////////////
// LRUCache<>::view
////////////////////////////////////////////////////////////////////////////////
template<typename KEY, typename DATA, typename HASH>
LRUCache<KEY, DATA, HASH>::view::view(Entry *entry)
    : entry(entry)
{}

template<typename KEY, typename DATA, typename HASH>
const KEY &LRUCache<KEY, DATA, HASH>::view::key() const
{
	return entry->key;
}

template<typename KEY, typename DATA, typename HASH>
const DATA &LRUCache<KEY, DATA, HASH>::view::data() const
{
	return entry->data;
}

////////////////////////////////////////////////////////////////////////////////
// LRUCache<>::iterator
////////////////////////////////////////////////////////////////////////////////
template<typename KEY, typename DATA, typename HASH>
LRUCache<KEY, DATA, HASH>::iterator::iterator(Entry *entry)
    : entry(entry)
{}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::view LRUCache<KEY, DATA, HASH>::iterator::operator*() const
{
	return view{ entry };
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::iterator &LRUCache<KEY, DATA, HASH>::iterator::operator++()
{
	entry = entry->next;
	return *this;
}

template<typename KEY, typename DATA, typename HASH>
bool LRUCache<KEY, DATA, HASH>::iterator::operator==(const iterator &rhs) const
{
	return entry == rhs.entry;
}

template<typename KEY, typename DATA, typename HASH>
bool LRUCache<KEY, DATA, HASH>::iterator::operator!=(const iterator &rhs) const
{
	return entry != rhs.entry;
}

////////////////////////////////////////////////////////////////////////////////
// LRUCache<>
////////////////////////////////////////////////////////////////////////////////
template<typename KEY, typename DATA, typename HASH>
LRUCache<KEY, DATA, HASH>::LRUCache(size_t capacity)
    : storage(capacity)
{
	for(size_t i = 0; i < capacity; i++)
	{
		Entry *entry = &storage[i];
		entry->next = free;  // No need for back link here.
		free = entry;
	}
}

template<typename KEY, typename DATA, typename HASH>
DATA LRUCache<KEY, DATA, HASH>::lookup(const Key &key)
{
	if(Entry *entry = find(key))
	{
		unlink(entry);
		link(entry);
		return entry->data;
	}
	return {};
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::add(const Key &key, const Data &data)
{
	if(Entry *entry = find(key))
	{
		// Move entry to front.
		unlink(entry);
		link(entry);
		entry->data = data;
		return;
	}

	Entry *entry = free;
	if(entry)
	{
		// Unlink from free.
		free = entry->next;
		entry->next = nullptr;
	}
	else
	{
		// Unlink least recently used.
		entry = tail;
		unlink(entry);
		set.erase(entry);
	}

	// link as most recently used.
	link(entry);
	if(tail == nullptr)
	{
		tail = entry;
	}

	entry->key = key;
	entry->data = data;
	set.emplace(entry);
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::clear()
{
	while(Entry *entry = head)
	{
		unlink(entry);
		entry->next = free;  // No need for back link here.
		free = entry;
	}
	set.clear();
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::iterator LRUCache<KEY, DATA, HASH>::begin() const
{
	return { head };
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::iterator LRUCache<KEY, DATA, HASH>::end() const
{
	return { nullptr };
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::unlink(Entry *entry)
{
	if(head == entry) { head = entry->next; }
	if(tail == entry) { tail = entry->prev; }
	if(entry->prev) { entry->prev->next = entry->next; }
	if(entry->next) { entry->next->prev = entry->prev; }
	entry->prev = nullptr;
	entry->next = nullptr;
}

template<typename KEY, typename DATA, typename HASH>
void LRUCache<KEY, DATA, HASH>::link(Entry *entry)
{
	ASSERT_MSG(entry->next == nullptr, "link() called on entry already linked");
	ASSERT_MSG(entry->prev == nullptr, "link() called on entry already linked");
	if(head)
	{
		entry->next = head;
		head->prev = entry;
	}
	head = entry;
	if(!tail) { tail = entry; }
}

template<typename KEY, typename DATA, typename HASH>
typename LRUCache<KEY, DATA, HASH>::Entry *LRUCache<KEY, DATA, HASH>::find(const Key &key)
{
	auto asKeyed = reinterpret_cast<const Keyed *>(&key);
	auto it = set.find(asKeyed);
	if(it == set.end())
	{
		return nullptr;
	}
	return const_cast<Entry *>(static_cast<const Entry *>(*it));
}

////////////////////////////////////////////////////////////////////////////////
// LRUCache<>::KeyedComparator
////////////////////////////////////////////////////////////////////////////////
template<typename KEY, typename DATA, typename HASH>
uint64_t LRUCache<KEY, DATA, HASH>::KeyedComparator::operator()(const Keyed *k) const
{
	return Hash()(k->key);
}

template<typename KEY, typename DATA, typename HASH>
uint64_t LRUCache<KEY, DATA, HASH>::KeyedComparator::operator()(const Keyed *a, const Keyed *b) const
{
	return a->key == b->key;
}

}  // namespace sw

#endif  // sw_LRUCache_hpp
