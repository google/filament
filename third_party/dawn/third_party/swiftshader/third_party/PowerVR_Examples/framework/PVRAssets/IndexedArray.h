/*!
\brief Implementation of a special kind of map that stores the data in a linear, contiguous store (std::vector interface),
but additionally contains an Index to them (std::map interface). Supports custom association of names with
values, and retrieval of indices by name or values by index.
\file         PVRAssets/IndexedArray.h
\author       PowerVR by Imagination, Developer Technology Team.
\copyright    Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include <vector>
#include <map>
#include <list>
#include <algorithm>
namespace pvr {
/// <summary>A combination of array (std::vector) with associative container (std::map). Supports association of
/// names with values, and retrieval by index.</summary>
/// <remarks>An std::vector style array class with the additional feature of associating "names" (IndexType_,
/// std::string by default) with the values stored. Keys are of type IndexType_, correspond to vector position 1:1,
/// so that each vector position ("index") is associated with a "key", and only one key. Use: Add pairs of values
/// with insert(key, value). Retrieve indices by key, using getIndex(key) -- O(logn) Retrieve values by index,
/// using indexing operator [] -- O(1) The remove() function destroys the items on which it was called, but a
/// default-constructed object will still exist. Performing insert() after removing an item will use the place of a
/// previously deleted item, if it exists. CAUTION: If remove() has been called, the vector no longer guarantees
/// contiguousness until compact() is called. CAUTION: To manually reclaim all memory and guarantee contiguous
/// allocation, call compact(). Calling compact invalidates all indices, which must then be retrieved anew by
/// "getInxdex". Calling getIndex on an unknown key returns (size_t)(-1) Accessing an unknown item by index is
/// undefined. Accessing an index not retrieved by getIndex since the last compact() operation is undefined.</remarks>
template<typename ValueType_, typename IndexType_ = std::string>
class IndexedArray
{
private:
	struct DictionaryEntry
	{
		ValueType_ value;
		IndexType_ key;
		DictionaryEntry() {}
		DictionaryEntry(const IndexType_& key, const ValueType_& value) : key(key), value(value) {}
	};
	struct StorageItem_ : DictionaryEntry
	{
		bool isUnused;
		StorageItem_() : isUnused(false) {}
		StorageItem_(const IndexType_& key, const ValueType_& value) : DictionaryEntry(key, value), isUnused(false) {}
	};

	typedef std::vector<StorageItem_> vectortype_;
	typedef typename std::map<IndexType_, size_t> maptype_;
	typedef std::list<size_t> deleteditemlisttype_;

	vectortype_ mystorage;
	maptype_ myindex;
	deleteditemlisttype_ myDeletedItems;

public:
	/// <summary>An (modifiable) Linear iterator of the IndexedArray class. Will linearly iterate the backing store
	/// skipping empy spots. Unordered.</summary>
	class iterator
	{
		friend class IndexedArray<ValueType_, IndexType_>;
		class const_iterator;
		StorageItem_* start;
		size_t current;
		size_t size; // required for out-of-bounds checks when skipping empty...
		iterator(StorageItem_* start, size_t size, size_t current = 0) : start(start), current(current), size(size) {}

	public:
		/// <summary>Copy constructor</summary>
		/// <param name="rhs">Object to copy</param>
		iterator(const const_iterator& rhs) : start(rhs.start), size(rhs.size), current(rhs.current) {}

		/// <summary>operator *</summary>
		/// <returns>Return this</returns>
		DictionaryEntry& operator*() { return *(start + current); }

		/// <summary>operator -></summary>
		/// <returns>Return this</returns>
		DictionaryEntry* operator->() { return (start + current); }

		/// <summary>Get the current index of the pointed-to item</summary>
		/// <returns>Current index</returns>
		size_t getItemIndex() { return current; }

		/// <summary>prefix operator ++</summary>
		/// <returns>This</returns>
		iterator& operator++()
		{
			while ((start + (++current))->isUnused && current != size) {} // skip empty values
			return *this;
		}

		/// <summary>postfix operator ++</summary>
		/// <returns>The value of this iterator prior to incrementing</returns>
		iterator operator++(int)
		{
			iterator ret = *this;
			while ((start + (++current))->isUnused && current != size) {} // skip empty values
			++(*this);
			return ret;
		}

		/// <summary>prefix operator --</summary>
		/// <returns>This</returns>
		iterator& operator--()
		{
			while ((start + (--current))->isUnused && current != static_cast<size_t>(-1)) {} // CAREFUL! this is a size_t, which means it WILL eventually overflow.
			return *this;
		}

		/// <summary>postfix operator --</summary>
		/// <returns>The value of this iterator prior to decrementing</returns>
		iterator operator--(int)
		{
			iterator ret = *this;
			--(*this);
			return ret;
		}

		/// <summary>operator !=</summary>
		/// <param name="rhs">Right hand side</param>
		/// <returns>True if this is not equal to rhs</returns>
		bool operator!=(const iterator& rhs) { return this->current != rhs.current; }

		/// <summary>operator ==</summary>
		/// <param name="rhs">Right hand side</param>
		/// <returns>True if this is equal to rhs</returns>
		bool operator==(const iterator& rhs) { return !((*this) != rhs); }
	};
	/// <summary>An (constant) Linear iterator of the IndexedArray class. Will linearly iterate the backing store
	/// skipping empy spots. Unordered.</summary>
	class const_iterator
	{
		friend class IndexedArray<ValueType_, IndexType_>;
		const StorageItem_* start;
		size_t current;
		size_t size; // required for out-of-bounds checks when skipping empty...
		const_iterator(const StorageItem_* start, size_t size, size_t current = 0) : start(start), size(size), current(current) {}

	public:
		/// <summary>Dereferencing operator: Return the object this iterator currently points to</summary>
		/// <returns>The object this iterator currently points to</returns>
		const DictionaryEntry& operator*() { return *(start + current); }
		/// <summary>Dereferencing operator: Access the members of the object this iterator currently points to</summary>
		/// <returns>A pointer to the object this iterator currently points to</returns>
		const DictionaryEntry* operator->() { return (start + current); }
		/// <summary>The index of the item where this iterator points</summary>
		/// <returns>The index of the item where this iterator points</returns>
		size_t getItemIndex() { return current; }
		/// <summary>prefix operator ++</summary>
		/// <returns>This</returns>
		const_iterator& operator++()
		{
			while (++current < size && (start + current)->isUnused) {} // skip empty values
			return *this;
		}

		/// <summary>postfix operator ++</summary>
		/// <returns>This</returns>
		const_iterator operator++(int)
		{
			const_iterator ret = *this;
			++(*this);
			return ret;
		}
		/// <summary>prefix operator --</summary>
		/// <returns>This</returns>
		const_iterator& operator--()
		{
			while (--current != static_cast<size_t>(-1) && (start + current)->isUnused) {} // CAREFUL! this is a size_t, which means it WILL eventually overflow.
			return *this;
		}

		/// <summary>postfix operator --</summary>
		/// <returns>This</returns>
		const_iterator operator--(int)
		{
			iterator ret = *this;
			--(*this);
			return ret;
		}

		/// <summary>Inequality.</summary>
		/// <param name="rhs">Right hand side of the operation</param>
		/// <returns>True if the items are not equal, otherwise false</returns>
		bool operator!=(const const_iterator& rhs) { return this->current != rhs.current; }

		/// <summary>Equality.</summary>
		/// <param name="rhs">Right hand side of the operation</param>
		/// <returns>True if the items are not equal, otherwise false</returns>
		bool operator==(const const_iterator& rhs) { return !((*this) != rhs); }
	};
	/// <summary>An Indexed iterator of the IndexedArray class. Will follow the indexing map of the IndexedArray
	/// iterating items in their Indexing order.</summary>
	typedef typename maptype_::iterator index_iterator;

	/// <summary>An Indexed (Constant) iterator of the IndexedArray class. Will follow the indexing map of the
	/// IndexedArray iterating items in their Indexing order.</summary>
	typedef typename maptype_::const_iterator const_index_iterator;

	/// <summary>Return a Linear iterator to the first non-deleted item in the backing store.</summary>
	/// <returns>A Linear iterator to the first non-deleted item in the backing store.</returns>
	iterator begin()
	{
		if (!mystorage.empty())
		{
			iterator ret(&mystorage.front(), mystorage.size());
			while ((ret.start + ret.current)->isUnused && (ret.current != ret.size)) { ++ret.current; } // skip empty values
			return ret;
		}
		else
		{
			return iterator(NULL, 0);
		}
	}

	/// <summary>Return a Linear const_iterator to the first non-deleted item in the backing store.</summary>
	/// <returns>A Linear const_iterator to the first non-deleted item in the backing store.</returns>
	const_iterator begin() const
	{
		if (!mystorage.empty())
		{
			const_iterator ret(&mystorage.front(), mystorage.size());
			while ((ret.start + ret.current)->isUnused && ret.current != ret.size) { ++ret.current; } // skip empty values
			return ret;
		}
		else
		{
			return const_iterator(NULL, 0);
		}
	}

	/// <summary>Return an indexed iterator by finding the provided key Indexing map.</summary>
	/// <param name="key">The key with which to lookup.</param>
	/// <returns>An indexed_iterator</returns>
	typename maptype_::iterator indexed_find(const IndexType_& key) { return myindex.find(key); }

	/// <summary>Return a const indexed iterator by finding the provided key Indexing map.</summary>
	/// <param name="key">The key with which to lookup.</param>
	/// <returns>A const indexed iterator</returns>
	typename maptype_::const_iterator indexed_find(const IndexType_& key) const { return myindex.find(key); }

	/// <summary>Return an indexed_const_iterator to the first item in the map.</summary>
	/// <returns>An indexed iterator to the beginning</returns>
	typename maptype_::iterator indexed_begin() { return myindex.begin(); }
	/// <summary>Return a indexed_const_iterator to the first item in the map.</summary>
	/// <returns>A constant indexed iterator to the beginning</returns>
	typename maptype_::const_iterator indexed_begin() const { return myindex.begin(); }

	/// <summary>Return an indexed_iterator pointing one item past the last item in the map.</summary>
	/// <returns>An indexed iterator to the end</returns>
	typename maptype_::iterator indexed_end() { return myindex.end(); }

	/// <summary>Return an indexed_const_iterator pointing one item past the last item in the map.</summary>
	/// <returns>A const indexed iterator to the end</returns>
	typename maptype_::const_iterator indexed_end() const { return myindex.end(); }

	/// <summary>Return an iterator pointing one item past the last item in the backing array.</summary>
	/// <returns>An iterator to the end of the backing array</returns>
	iterator end() { return mystorage.empty() ? iterator(NULL, 0) : iterator(&mystorage.front(), mystorage.size(), mystorage.size()); }

	/// <summary>Return a const_iterator pointing one item past the last item in the backing array.</summary>
	/// <returns>An iterator to the end of the backing array</returns>
	const_iterator end() const { return mystorage.empty() ? const_iterator(NULL, 0) : const_iterator(&mystorage.front(), mystorage.size(), mystorage.size()); }

	/// <summary>Insert an item at a specific point in the backing array.</summary>
	/// <param name="where">The index where to insert the new item</param>
	/// <param name="key">The Key of the new item</param>
	/// <param name="val">The Value of the new item</param>
	void insertAt(size_t where, const IndexType_& key, const ValueType_& val)
	{
		if (insert(key, val) != where) { relocate(key, where); }
	}

	/// <summary>Insert an item at the first possible spot in the backing array. If key exists, will update the value.</summary>
	/// <param name="key">The Key of the new item</param>
	/// <param name="val">The Value of the new item</param>
	/// <returns>The index, in the backing array, of the inserted/updated item</returns>
	size_t insert(const IndexType_& key, const ValueType_& val)
	{
		std::pair<typename maptype_::iterator, bool> found = myindex.insert(std::make_pair(key, 0));
		if (!found.second) // Element already existed!
		{ mystorage[found.first->second].value = val; } else
		{
			found.first->second = insertinvector(key, val);
		}
		return found.first->second;
	}

	/// <summary>Get the index of a specific key in the backing array. Valid until a reshuffling of the array is done
	/// via insert, compact or similar operation.</summary>
	/// <param name="key">The Key of the item to find</param>
	/// <returns>The index of <paramRef name="key"/>, -1 if key does not exist.</returns>
	size_t getIndex(const IndexType_& key) const
	{
		typename maptype_::const_iterator found = myindex.find(key);
		if (found != myindex.end()) // Element already existed!
		{ return found->second; } return static_cast<size_t>(-1);
	}

	/// <summary>Removes the item with the specified key from the IndexedArray.</summary>
	/// <param name="key">The Key of the item to erase</param>
	/// <remarks>This method will find the entry with specified key and remove it. It will not invalidata existing
	/// indices, but it will void the contiguousness guarantee the backing array normally has. Call compact()
	/// afterwards to make the vector contiguous again (but invalidate existing indices).</remarks>
	void erase(const IndexType_& key)
	{
		typename maptype_::iterator where = myindex.find(key);
		if (where != myindex.end())
		{
			removefromvector(where->second);
			myindex.erase(where);

			// SPECIAL CASE: If no more items are left, there is absolutely no point in NOT compacting, as no iterators or indices exist to be invalidated, so we can clean up
			// even though "deferred" was asked. Additionally, this is essentially free, except maybe for the list...
			if (myindex.empty())
			{
				mystorage.clear();
				myDeletedItems.clear();
			}
		}
	}

	/// <summary>Array indexing operator. Constant time. Use getIndex to get the indices of specific items.
	/// If idx points to a deleted item or past the last item, the behaviour is undefined.</summary>
	/// <param name="idx">The backing index to retrieve.</param>
	/// <returns>Reference to the item at specified index</returns>
	ValueType_& operator[](size_t idx) { return mystorage[idx].value; }
	/// <summary>Const array indexing operator. Use getIndex to get the indices of specific items. If idx points to a
	/// deleted item or past the last item, the behaviour is undefined.</summary>
	/// <param name="idx">The backing index to retrieve.</param>
	/// <returns>Const Reference to the item at specified index</returns>
	const ValueType_& operator[](size_t idx) const { return mystorage[idx].value; }

	/// <summary>Indexed indexing operator. Uses std::map find to retrieve the specified value. If the
	/// key does not exist, it will be inserted.</summary>
	/// <param name="key">The key to find.</param>
	/// <returns> Reference to the item at specified index</returns>
	ValueType_& operator[](const IndexType_& key) { return mystorage[myindex.find(key)->second].value; }

	/// <summary>Indexed indexing operator. Uses std::map find to retrieve the specified value. If the
	/// key does not exist, it will be inserted.</summary>
	/// <param name="key">The key to find.</param>
	/// <returns> Reference to the item at specified index</returns>
	const ValueType_& operator[](const IndexType_& key) const { return mystorage[myindex.find(key)->second].value; }

	/// <summary>Compacts the backing array by removing existing items from the end of the vector and putting them in the
	/// place of deleted items, and then updating their index, until no more positions marked as deleted are left.
	/// Will ensure the contiguousness of the backing vector, but will invalidate previously gotten item indices.</summary>
	void compact()
	{
		// We can do that because the last remove() tears down all datastructures used.
		if (!myindex.size()) { return; }
		// First, make sure there is something to compact...
		deleteditemlisttype_::iterator unused_spot = myDeletedItems.begin();
		while (myDeletedItems.size() && unused_spot != myDeletedItems.end())
		{
			// Last item in the storage vector.
			size_t last = mystorage.size();

			// 1) Trim the end of the vector...
			while (last-- && mystorage[last].isUnused)
			{
				mystorage.pop_back();
				// Rinse, repeat. If size is zero, there is nothing to do.
			}

			// Either the storage is empty
			if (mystorage.empty())
			{
				myDeletedItems.clear();
				// the rest should already be cleared...
				// the loop will exit naturally
			}
			else // Or we can have find its last item!
			{
				// 2)Trim any items from the end of the unused spots list that may have been trimmed off by the vector...
				while (unused_spot != myDeletedItems.end() && *unused_spot >= last) { myDeletedItems.erase(unused_spot++); }
				// Any spots left?
				if (unused_spot != myDeletedItems.end())
				{
					// Do the actual data movement. After all we've been through, we know that
					// i. The last item of the vector is a valid item (guaranteed by 1)
					// ii. The unused spot is not out of bounds of the vector
					// iii.
					// Copy by hand(as we have not defined a move assignment operator for compatibility reasons.
					// Also, since the std::string does not throw, this makes easier to reason about exceptions.

					mystorage[*unused_spot].value = std::move(mystorage[last].value);
					mystorage[*unused_spot].key = std::move(mystorage[last].key);

					mystorage[*unused_spot].isUnused = false;
					myindex[mystorage[*unused_spot].key] = *unused_spot;
					mystorage.pop_back();
					myDeletedItems.erase(unused_spot++);
				} // else : No action needed - unused spots has been trimmed off completely, so no movement is possible, or necessary...
			}
		}
	}

	/// <summary>Empties the IndexedArray.</summary>
	void clear()
	{
		myindex.clear();
		mystorage.clear();
		myDeletedItems.clear();
	}

	/// <summary>Gets the number of items in the IndexedArray.</summary>
	/// <returns>The number of items in the IndexedArray.</returns>
	size_t size() const { return myindex.size(); }

	/// <summary>Gets the number of items in the IndexedArray, including items that have been deleted.</summary>
	/// <returns>The number of items in the IndexedArray, including items that have been deleted.</returns>
	size_t sizeWithDeleted() const { return mystorage.size(); }

	/// <summary>Gets the current capacity of the backing array of the IndexedArray.</summary>
	/// <returns>The current capacity of the backing array of the IndexedArray.</returns>
	size_t capacity() const { return mystorage.size(); }

	/// <summary>Gets the number of deleted items.</summary>
	/// <returns>The number of deleted items.</returns>
	size_t deletedItemsCount() const { return myDeletedItems.size(); }

	/// <summary>Move a specific item (identified by a key) to a specific index in the list. If an item is already in
	/// this spot in the list, their positions are swapped.</summary>
	/// <param name="key">The key of the item to relocate.</param>
	/// <param name="index">The index where to relocate the item.</param>
	/// <returns>False if the specified key was not found in the index.</returns>
	bool relocate(const IndexType_& key, size_t index)
	{
		typename maptype_::iterator found = myindex.find(key);
		if (found == myindex.end()) { return false; }
		size_t old_index = myindex[key];
		if (index == old_index) { return true; } // No-op
		if (index + 1 > mystorage.size()) // Storage not big enough.
		{
			// Grow, and mark unused all required items. Need to add all spots (But the last) to unusedspots.
			size_t oldsize = mystorage.size();
			mystorage.resize(index + 1);
			for (size_t i = oldsize; i < index; ++i)
			{
				myDeletedItems.push_front(i);
				mystorage[i].isUnused = 1;
			}
			mystorage.back() = mystorage[old_index];
			removefromvector(old_index);
		}
		else if (mystorage[index].isUnused) // Lucky! Storage is big enough, and the item is not used!
		{
			deleteditemlisttype_::iterator place = std::find(myDeletedItems.begin(), myDeletedItems.end(), index);
			assert(place != myDeletedItems.end()); // Shouldn't happen! Ever!
			myDeletedItems.erase(place);
			mystorage[index] = mystorage[old_index];
			removefromvector(old_index);
		}
		else // Whoops! Space is already occupied. Swap with the old item!
		{
			myindex[mystorage[index].key] = old_index;
			std::swap(mystorage[index], mystorage[old_index]);
		}
		myindex[key] = index;
		return true;
	}

private:
	size_t insertinvector(const IndexType_& key, const ValueType_& val)
	{
		size_t retval;
		if (myDeletedItems.empty())
		{
			retval = mystorage.size();
			mystorage.emplace_back(StorageItem_());
			mystorage.back().isUnused = false;
			mystorage.back().key = key;
			mystorage.back().value = val;
		}
		else
		{
			retval = myDeletedItems.back();
			myDeletedItems.pop_back();
			mystorage[retval].value = val;
			mystorage[retval].key = key;
			mystorage[retval].isUnused = false;
		}
		return retval;
	}
	void removefromvector(size_t index)
	{
		if (index == (mystorage.size() - 1))
		{
			// Removing the last item from the vector -- just pop it.
			mystorage.pop_back();
		}
		else
		{
			// NOT the last item, so we just destruct it (to free any potential expensive resources),
			// and then default-construct it (to have the spot destructible). We keep the reference to
			// the key as we will need it.
			myDeletedItems.push_front(index);
			mystorage[index].isUnused = true;
			mystorage[index].value.ValueType_::~ValueType_();
			new (&mystorage[index].value) ValueType_;
		}
	}
};
} // namespace pvr
