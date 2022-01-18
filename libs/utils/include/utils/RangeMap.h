/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_UTILS_RANGEMAP_H
#define TNT_UTILS_RANGEMAP_H

#include <utils/Panic.h>
#include <utils/Range.h>
#include <utils/debug.h>

#include <map>

namespace utils {

/**
 * Sparse container for a series of ordered non-overlapping intervals.
 *
 * RangeMap has a low memory footprint if it contains fairly homogeneous data. Internally, the
 * intervals are automatically split and merged as elements are added or removed.
 *
 * Each interval maps to an instance of ValueType, which should support cheap equality checks
 * and copy assignment. (simple concrete types are ideal)
 *
 * KeyType should support operator< because intervals are internally sorted using std::map.
 */
template<typename KeyType, typename ValueType>
class RangeMap {
public:
    /**
     *  Replaces all slots between first (inclusive) and last (exclusive).
     */
    void add(KeyType first, KeyType last, const ValueType& value) noexcept {
        // First check if an existing range contains "first".
        iterator iter = find(first);
        if (iter != end()) {
            Range<KeyType>& existing = get_range(iter);
            // Check if the existing range be extended.
            if (get_value(iter) == value) {
                if (existing.last < last) {
                    wipe(existing.last, last);
                    existing.last = last;
                }
                return;
            }
            // Split the existing range into two ranges.
            if (last < existing.last && first > existing.first) {
                const KeyType tmp = existing.last;
                existing.last = first;
                insert(first, last, value);
                insert(last, tmp, get_value(iter));
                return;
            }
            // Clip the end of the existing range and potentially remove it.
            existing.last = first;
            if (existing.empty()) {
                mMap.erase(iter);
            }
            wipe(first, last);
            insert(first, last, value);
            return;
        }

        // Check if an existing range contains the end of the new range.
        KeyType back = last;
        iter = find(--back);
        if (iter == end()) {
            wipe(first, last);
            insert(first, last, value);
            return;
        }
        Range<KeyType>& existing = get_range(iter);

        // Check if the existing range be extended.
        if (get_value(iter) == value) {
            if (existing.first > first) {
                wipe(first, existing.first);
                existing.first = first;
            }
            return;
        }

        // Clip the beginning of the existing range and potentially remove it.
        existing.first = last;
        if (existing.empty()) {
            mMap.erase(iter);
        }
        wipe(first, last);
        insert(first, last, value);
    }

    /**
     * Shorthand for the "add" method that inserts a single element.
     */
    void set(KeyType key, const ValueType& value) noexcept {
        KeyType begin = key;
        add(begin, ++key, value);
    }

    /**
     * Checks if a range exists that encompasses the given key.
     */
    bool has(KeyType key) const noexcept {
        return find(key) != mMap.end();
    }

    /**
     * Retrieves the element at the given location, panics if no element exists.
     */
    const ValueType& get(KeyType key) const {
        const_iterator iter = find(key);
        ASSERT_PRECONDITION(iter != end(), "RangeMap: No element exists at the given key.");
        return get_value(iter);
    }

    /**
     * Removes all elements between begin (inclusive) and end (exclusive).
     */
    void clear(KeyType first, KeyType last) noexcept {
        // Check if an existing range contains "first".
        iterator iter = find(first);
        if (iter != end()) {
            Range<KeyType>& existing = get_range(iter);
            // Split the existing range into two ranges.
            if (last < existing.last && first > existing.first) {
                const KeyType tmp = existing.last;
                existing.last = first;
                insert(last, tmp, get_value(iter));
                return;
            }
            // Clip the end of the existing range and potentially remove it.
            existing.last = first;
            if (existing.empty()) {
                mMap.erase(iter);
            }
            wipe(first, last);
            return;
        }

        // Check if an existing range contains the end of the new range.
        KeyType back = last;
        iter = find(--back);
        if (iter == end()) {
            wipe(first, last);
            return;
        }
        Range<KeyType>& existing = get_range(iter);

        // Clip the beginning of the existing range and potentially remove it.
        existing.first = last;
        if (existing.empty()) {
            mMap.erase(iter);
        }
        wipe(first, last);
    }

    /**
     * Shorthand for the "clear" method that clears a single element.
     */
    void reset(KeyType key) noexcept {
        KeyType begin = key;
        clear(begin, ++key);
    }

    /**
     * Returns the number of internal interval objects (rarely used).
     */
    size_t range_count() const noexcept { return mMap.size(); }

private:

    using Map = std::map<KeyType, std::pair<Range<KeyType>, ValueType>>;
    using iterator = typename Map::iterator;
    using const_iterator = typename Map::const_iterator;

    const_iterator begin() const noexcept { return mMap.begin(); }
    const_iterator end() const noexcept { return mMap.end(); }

    iterator begin() noexcept { return mMap.begin(); }
    iterator end() noexcept { return mMap.end(); }

    Range<KeyType>& get_range(iterator iter) const { return iter->second.first; }
    ValueType& get_value(iterator iter) const { return iter->second.second; }

    const Range<KeyType>& get_range(const_iterator iter) const { return iter->second.first; }
    const ValueType& get_value(const_iterator iter) const { return iter->second.second; }

    // Private helper that assumes there is no existing range that overlaps the given range.
    void insert(KeyType first, KeyType last, const ValueType& value) noexcept {
        assert_invariant(!has(first));
        assert_invariant(!has(last - 1));

        // Check if there is an adjacent range to the left than can be extended.
        KeyType previous = first;
        if (iterator iter = find(--previous); iter != end() && get_value(iter) == value) {
            get_range(iter).last = last;
            return;
        }

        // Check if there is an adjacent range to the right than can be extended.
        if (iterator iter = find(last); iter != end() && get_value(iter) == value) {
            get_range(iter).first = first;
            return;
        }

        mMap[first] = {Range<KeyType> { first, last }, value};
    }

    // Private helper that erases all intervals that are wholly contained within the given range.
    // Note that this is quite different from the public "clear" method.
    void wipe(KeyType first, KeyType last) noexcept {
        // Find the first range whose beginning is greater than or equal to "first".
        iterator iter = mMap.lower_bound(first);
        while (iter != end() && get_range(iter).first < last) {
            KeyType existing_last = get_range(iter).last;
            if (existing_last > last) {
                break;
            }
            iter = mMap.erase(iter);
        }
    }

    const_iterator find(KeyType key) const noexcept {
        // Find the first range whose beginning is greater than or equal to the given key.
        const_iterator iter = mMap.lower_bound(key);
        if (iter != end() && get_range(iter).contains(key)) {
            return iter;
        }
        // If that was the first range, or if the map is empty, return false.
        if (iter == begin()) {
            return end();
        }
        // Check the range immediately previous to the one that was found.
        return get_range(--iter).contains(key) ? iter : end();
    }

    iterator find(KeyType key) noexcept {
        // Find the first range whose beginning is greater than or equal to the given key.
        iterator iter = mMap.lower_bound(key);
        if (iter != end() && get_range(iter).contains(key)) {
            return iter;
        }
        // If that was the first range, or if the map is empty, return false.
        if (iter == begin()) {
            return end();
        }
        // Check the range immediately previous to the one that was found.
        return get_range(--iter).contains(key) ? iter : end();
    }

    // This maps from the start value of each range to the range itself.
    Map mMap;
};

} // namespace utils

#endif // TNT_UTILS_RANGEMAP_H
