// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_UTILS_CONTAINERS_UNIQUE_VECTOR_H_
#define SRC_TINT_UTILS_CONTAINERS_UNIQUE_VECTOR_H_

#include <cstddef>
#include <functional>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/utils/containers/hashset.h"
#include "src/tint/utils/containers/vector.h"

namespace tint {

/// UniqueVector is an ordered container that only contains unique items.
/// Attempting to add a duplicate is a no-op.
template <typename T, size_t N, typename HASH = Hasher<T>, typename EQUAL = std::equal_to<T>>
struct UniqueVector {
    /// STL-friendly alias to T. Used by gmock.
    using value_type = T;

    /// Constructor
    UniqueVector() = default;

    /// Constructor
    /// @param v the vector to construct this UniqueVector with. Duplicate
    /// elements will be removed.
    explicit UniqueVector(std::vector<T>&& v) {
        for (auto& el : v) {
            Add(el);
        }
    }

    /// Add appends the item to the end of the vector, if the vector does not
    /// already contain the given item.
    /// @param item the item to append to the end of the vector
    /// @returns true if the item was added, otherwise false.
    bool Add(const T& item) {
        if (set.Add(item)) {
            vector.Push(item);
            return true;
        }
        return false;
    }

    /// Removes @p count elements from the vector
    /// @param start the index of the first element to remove
    /// @param count the number of elements to remove
    void Erase(size_t start, size_t count = 1) {
        for (size_t i = 0; i < count; i++) {
            set.Remove(vector[start + i]);
        }
        vector.Erase(start, count);
    }

    /// @returns true if the vector contains `item`
    /// @param item the item
    bool Contains(const T& item) const { return set.Contains(item); }

    /// @param i the index of the element to retrieve
    /// @returns the element at the index `i`
    T& operator[](size_t i) { return vector[i]; }

    /// @param i the index of the element to retrieve
    /// @returns the element at the index `i`
    const T& operator[](size_t i) const { return vector[i]; }

    /// @returns true if the vector is empty
    bool IsEmpty() const { return vector.IsEmpty(); }

    /// Removes all elements from the vector
    void Clear() {
        vector.Clear();
        set.Clear();
    }

    /// @returns the number of items in the vector
    size_t Length() const { return vector.Length(); }

    /// @returns the pointer to the first element in the vector, or nullptr if the vector is empty.
    const T* Data() const { return vector.IsEmpty() ? nullptr : &vector[0]; }

    /// @returns an iterator to the beginning of the vector
    auto begin() const { return vector.begin(); }

    /// @returns an iterator to the end of the vector
    auto end() const { return vector.end(); }

    /// @returns an iterator to the beginning of the reversed vector
    auto rbegin() const { return vector.rbegin(); }

    /// @returns an iterator to the end of the reversed vector
    auto rend() const { return vector.rend(); }

    /// @returns a reference to the internal vector
    operator VectorRef<T>() const { return vector; }

    /// @returns the std::move()'d vector.
    /// @note The UniqueVector must not be used after calling this method
    VectorRef<T> Release() { return std::move(vector); }

    /// Pre-allocates `count` elements in the vector and set
    /// @param count the number of elements to pre-allocate
    void Reserve(size_t count) {
        vector.Reserve(count);
        set.Reserve(count);
    }

    /// Removes the last element from the vector
    /// @returns the popped element
    T Pop() {
        set.Remove(vector.Back());
        return vector.Pop();
    }

    /// Removes the last element from the vector if it is equal to @p value
    /// @param value the value to pop if it is at the back of the vector
    /// @returns true if the value was popped, otherwise false
    bool TryPop(T value) {
        if (!vector.IsEmpty() && vector.Back() == value) {
            set.Remove(vector.Back());
            vector.Pop();
            return true;
        }
        return false;
    }

  private:
    Vector<T, N> vector;
    Hashset<T, N, HASH, EQUAL> set;
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_CONTAINERS_UNIQUE_VECTOR_H_
