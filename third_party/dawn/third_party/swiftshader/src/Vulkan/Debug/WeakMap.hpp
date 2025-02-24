// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#ifndef VK_DEBUG_WEAKMAP_HPP_
#define VK_DEBUG_WEAKMAP_HPP_

#include <map>
#include <memory>

namespace vk {
namespace dbg {

// WeakMap is an associative container of keys of type K to values of type
// std::weak_ptr<V>.
// WeakMap's iterators will skip over elements where the value has no more
// remaining std::shared_ptr<V> references.
// WeakMap is not thread-safe and requires the use of an external mutex to be
// used by multiple threads, concurrently.
template<typename K, typename V>
class WeakMap
{
	using Map = std::map<K, std::weak_ptr<V>>;
	using MapIterator = typename Map::const_iterator;

public:
	class iterator
	{
	public:
		inline iterator(const MapIterator &it, const MapIterator &end);
		inline void operator++();
		inline bool operator==(const iterator &) const;
		inline bool operator!=(const iterator &) const;
		inline std::pair<K, std::shared_ptr<V>> operator*() const;

	private:
		void skipNull();

		MapIterator it;
		const MapIterator end;
		std::shared_ptr<V> sptr;
	};

	// begin() returns an iterator to the start of the map.
	inline iterator begin() const;

	// end() returns an iterator to the end of the map.
	inline iterator end() const;

	// approx_size() returns an approximate number of entries in the map. This
	// is guaranteed to be greater than or equal to the actual number of
	// elements in the map.
	inline size_t approx_size() const;

	// get() returns the std::shared_ptr<V> value for the given key, or nullptr
	// if the map does not contain the key, or the last remaining
	// std::shared_ptr<V> reference to the value has been dropped.
	inline std::shared_ptr<V> get(const K &key) const;

	// add() attempts to insert the key-value pair into the map.
	// add() returns true if there was no existing entry with the given key,
	// and the pair was added, otherwise false.
	inline bool add(const K &key, const std::shared_ptr<V> &val);

	// remove() attempts to remove the entry with the given key from the map.
	// remove() returns true if there was no existing entry with the given key,
	// and the entry was removed, otherwise false.
	inline bool remove(const K &key);

private:
	// reap() removes any entries that have values with no external references.
	inline void reap();

	Map map;
	size_t reapAtSize = 32;
};

template<typename K, typename V>
WeakMap<K, V>::iterator::iterator(const MapIterator &it, const MapIterator &end)
    : it(it)
    , end(end)
{
	skipNull();
}

template<typename K, typename V>
void WeakMap<K, V>::iterator::operator++()
{
	it++;
	skipNull();
}

template<typename K, typename V>
void WeakMap<K, V>::iterator::skipNull()
{
	for(; it != end; ++it)
	{
		// Hold on to the shared_ptr when pointing at this map element.
		// This ensures that the object is not released.
		sptr = it->second.lock();
		if(sptr)
		{
			return;
		}
	}
}

template<typename K, typename V>
bool WeakMap<K, V>::iterator::operator==(const iterator &rhs) const
{
	return it == rhs.it;
}

template<typename K, typename V>
bool WeakMap<K, V>::iterator::operator!=(const iterator &rhs) const
{
	return it != rhs.it;
}

template<typename K, typename V>
std::pair<K, std::shared_ptr<V>> WeakMap<K, V>::iterator::operator*() const
{
	return { it->first, sptr };
}

template<typename K, typename V>
typename WeakMap<K, V>::iterator WeakMap<K, V>::begin() const
{
	return iterator(map.begin(), map.end());
}

template<typename K, typename V>
typename WeakMap<K, V>::iterator WeakMap<K, V>::end() const
{
	return iterator(map.end(), map.end());
}

template<typename K, typename V>
size_t WeakMap<K, V>::approx_size() const
{
	return map.size();
}

template<typename K, typename V>
std::shared_ptr<V> WeakMap<K, V>::get(const K &key) const
{
	auto it = map.find(key);
	return (it != map.end()) ? it->second.lock() : nullptr;
}

template<typename K, typename V>
bool WeakMap<K, V>::add(const K &key, const std::shared_ptr<V> &val)
{
	if(map.size() > reapAtSize)
	{
		reap();
		reapAtSize = map.size() * 2 + 32;
	}
	return map.emplace(key, val).second;
}

template<typename K, typename V>
bool WeakMap<K, V>::remove(const K &key)
{
	return map.erase(key) > 0;
}

template<typename K, typename V>
void WeakMap<K, V>::reap()
{
	for(auto it = map.begin(); it != map.end();)
	{
		if(it->second.expired())
		{
			map.erase(it++);
		}
		else
		{
			++it;
		}
	}
}

}  // namespace dbg
}  // namespace vk

#endif  // VK_DEBUG_WEAKMAP_HPP_
