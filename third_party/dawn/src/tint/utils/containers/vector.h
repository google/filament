// Copyright 2022 The Dawn & Tint Authors
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

#ifndef SRC_TINT_UTILS_CONTAINERS_VECTOR_H_
#define SRC_TINT_UTILS_CONTAINERS_VECTOR_H_

#include <stddef.h>
#include <stdint.h>
#include <algorithm>
#include <atomic>
#include <iterator>
#include <new>
#include <utility>
#include <vector>

#include "src/tint/utils/containers/slice.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/memory/aligned_storage.h"
#include "src/tint/utils/memory/bitcast.h"

TINT_BEGIN_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

#ifndef TINT_VECTOR_MUTATION_CHECKS_ENABLED
#ifdef NDEBUG
#define TINT_VECTOR_MUTATION_CHECKS_ENABLED 0
#else
#define TINT_VECTOR_MUTATION_CHECKS_ENABLED 1
#endif
#endif

#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
#define TINT_VECTOR_MUTATION_CHECK_ASSERT(x) TINT_ASSERT(x)
#else
#define TINT_VECTOR_MUTATION_CHECK_ASSERT(x)
#endif

/// Forward declarations
namespace tint {
template <typename>
class VectorRef;
}  // namespace tint

namespace tint {

/// VectorIterator is a forward iterator of Vector elements.
template <typename T, bool FORWARD = true>
class VectorIterator {
  public:
    /// The iterator trait
    using iterator_category = std::random_access_iterator_tag;
    /// The type of an element that this iterator points to
    using value_type = T;
    /// The type of the difference of two iterators
    using difference_type = std::ptrdiff_t;
    /// A pointer of the element type
    using pointer = T*;
    /// A reference of the element type
    using reference = T&;

    /// Constructor
    VectorIterator() = default;

    /// Destructor
    ~VectorIterator() {
#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
        if (iterator_count_) {
            TINT_ASSERT(*iterator_count_ > 0);
            (*iterator_count_)--;
        }
#endif
    }

#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
    /// Constructor
    /// @param p the pointer to the vector element
    /// @param it_cnt a pointer to an iterator count
    VectorIterator(T* p, std::atomic<uint32_t>* it_cnt) : ptr_(p), iterator_count_(it_cnt) {
        (*iterator_count_)++;
    }

    /// Copy constructor
    /// @param other the VectorIterator to copy
    VectorIterator(const VectorIterator& other)
        : ptr_(other.ptr_), iterator_count_(other.iterator_count_) {
        if (iterator_count_) {
            (*iterator_count_)++;
        }
    }

    /// Move constructor
    /// @param other the VectorIterator to move
    VectorIterator(VectorIterator&& other)
        : ptr_(other.ptr_), iterator_count_(other.iterator_count_) {
        other.ptr_ = nullptr;
        other.iterator_count_ = nullptr;
    }
#else
    /// Constructor
    /// @param p the pointer to the vector element
    explicit VectorIterator(T* p) : ptr_(p) {}

    /// Copy constructor
    /// @param other the VectorIterator to copy
    VectorIterator(const VectorIterator& other) : ptr_(other.ptr_) {}

    /// Move constructor
    /// @param other the VectorIterator to move
    VectorIterator(VectorIterator&& other) : ptr_(other.ptr_) { other.ptr_ = nullptr; }
#endif

    /// Assignment operator
    /// @param other the VectorIterator to copy
    /// @return this VectorIterator
    VectorIterator& operator=(const VectorIterator& other) {
        ptr_ = other.ptr_;
#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
        if (iterator_count_ != other.iterator_count_) {
            if (iterator_count_) {
                (*iterator_count_)--;
            }
            iterator_count_ = other.iterator_count_;
            if (iterator_count_) {
                (*iterator_count_)++;
            }
        }
#endif
        return *this;
    }

    /// Move-assignment operator
    /// @param other the VectorIterator to move
    /// @return this VectorIterator
    VectorIterator& operator=(VectorIterator&& other) {
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
        if (iterator_count_) {
            (*iterator_count_)--;
        }
        iterator_count_ = other.iterator_count_;
        other.iterator_count_ = nullptr;
#endif
        return *this;
    }

    /// @return the element this iterator currently points at
    operator T*() const { return ptr_; }

    /// @return the element this iterator currently points at
    T& operator*() const { return *ptr_; }

    /// @return the element this iterator currently points at
    T* operator->() const { return ptr_; }

    /// Equality operator
    /// @param other the other VectorIterator
    /// @return true if this iterator is equal to @p other
    bool operator==(const VectorIterator& other) const { return ptr_ == other.ptr_; }

    /// Inequality operator
    /// @param other the other VectorIterator
    /// @return true if this iterator is not equal to @p other
    bool operator!=(const VectorIterator& other) const { return ptr_ != other.ptr_; }

    /// Less-than operator
    /// @param other the other iterator
    /// @returns true if this iterator comes before @p other
    bool operator<(const VectorIterator& other) const { return other - *this > 0; }

    /// Greater-than operator
    /// @param other the other iterator
    /// @returns true if this iterator comes after @p other
    bool operator>(const VectorIterator& other) const { return *this - other > 0; }

    /// Index operator
    /// @param i the number of elements from the element this iterator points to
    /// @return the element
    T& operator[](std::ptrdiff_t i) const { return *(*this + i); }

    /// Increments the iterator (prefix)
    /// @returns this VectorIterator
    VectorIterator& operator++() {
        this->ptr_ = FORWARD ? this->ptr_ + 1 : this->ptr_ - 1;
        return *this;
    }

    /// Decrements the iterator (prefix)
    /// @returns this VectorIterator
    VectorIterator& operator--() {
        this->ptr_ = FORWARD ? this->ptr_ - 1 : this->ptr_ + 1;
        return *this;
    }

    /// Increments the iterator (postfix)
    /// @returns a VectorIterator that points to the element before the increment
    VectorIterator operator++(int) {
        VectorIterator res = *this;
        this->ptr_ = FORWARD ? this->ptr_ + 1 : this->ptr_ - 1;
        return res;
    }

    /// Decrements the iterator (postfix)
    /// @returns a VectorIterator that points to the element before the decrement
    VectorIterator operator--(int) {
        VectorIterator res = *this;
        this->ptr_ = FORWARD ? this->ptr_ - 1 : this->ptr_ + 1;
        return res;
    }

    /// Moves the iterator forward by @p n elements
    /// @param n the number of elements
    /// @returns this VectorIterator
    VectorIterator operator+=(std::ptrdiff_t n) {
        this->ptr_ = FORWARD ? this->ptr_ + n : this->ptr_ - n;
        return *this;
    }

    /// Moves the iterator backwards by @p n elements
    /// @param n the number of elements
    /// @returns this VectorIterator
    VectorIterator operator-=(std::ptrdiff_t n) {
        this->ptr_ = FORWARD ? this->ptr_ - n : this->ptr_ + n;
        return *this;
    }

    /// @param n the number of elements
    /// @returns a new VectorIterator progressed by @p n elements
    VectorIterator operator+(std::ptrdiff_t n) const {
#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
        return VectorIterator{FORWARD ? ptr_ + n : ptr_ - n, iterator_count_};
#else
        return VectorIterator{FORWARD ? ptr_ + n : ptr_ - n};
#endif
    }

    /// @param n the number of elements
    /// @returns a new VectorIterator regressed by @p n elements
    VectorIterator operator-(std::ptrdiff_t n) const {
#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
        return VectorIterator{FORWARD ? ptr_ - n : ptr_ + n, iterator_count_};
#else
        return VectorIterator{FORWARD ? ptr_ - n : ptr_ + n};
#endif
    }

    /// @param other the other iterator
    /// @returns the number of elements between this iterator and @p other
    std::ptrdiff_t operator-(const VectorIterator& other) const {
        return FORWARD ? ptr_ - other.ptr_ : other.ptr_ - ptr_;
    }

  private:
    T* ptr_ = nullptr;
#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
    std::atomic<uint32_t>* iterator_count_ = nullptr;
#endif
};

/// @param out the stream to write to
/// @param it the VectorIterator
/// @returns @p out so calls can be chained
template <typename STREAM, typename T, bool FORWARD, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& out, const VectorIterator<T, FORWARD>& it) {
    return out << *it;
}

/// Vector is a small-object-optimized, dynamically-sized vector of contigious elements of type T.
///
/// Vector will fit `N` elements internally before spilling to heap allocations. If `N` is greater
/// than zero, the internal elements are stored in a 'small array' held internally by the Vector.
///
/// Vectors can be copied or moved.
///
/// Copying a vector will either copy to the 'small array' if the number of elements is equal to or
/// less than N, otherwise elements will be copied into a new heap allocation.
///
/// Moving a vector will reassign ownership of the heap-allocation memory, if the source vector
/// holds its elements in a heap allocation, otherwise a copy will be made as described above.
///
/// Vector is optimized for CPU performance over memory efficiency. For example:
/// * Moving a vector that stores its elements in a heap allocation to another vector will simply
///   assign the heap allocation, even if the target vector can hold the elements in its 'small
///   array'. This reduces memory copying, but may incur additional memory usage.
/// * Resizing, or popping elements from a vector that has spilled to a heap allocation does not
///   revert back to using the 'small array'. Again, this is to reduce memory copying.
template <typename T, size_t N>
class Vector {
  public:
    /// Alias to the non-const forward iterator
    using iterator = VectorIterator<T, /* forward */ true>;
    /// Alias to the const forward iterator
    using const_iterator = VectorIterator<const T, /* forward */ true>;
    /// Alias to the non-const reverse  iterator
    using reverse_iterator = VectorIterator<T, /* forward */ false>;
    /// Alias to the const reverse iterator
    using const_reverse_iterator = VectorIterator<const T, /* forward */ false>;
    /// Alias to `T`.
    using value_type = T;
    /// Value of `N`
    static constexpr size_t static_length = N;

    /// Constructor
    Vector() = default;

    /// Constructor
    Vector(EmptyType) {}  // NOLINT(runtime/explicit)

    /// Constructor
    /// @param elements the elements to place into the vector
    Vector(std::initializer_list<T> elements) {
        Reserve(elements.size());
        for (auto& el : elements) {
            new (&impl_.slice.data[impl_.slice.len++]) T{el};
        }
    }

    /// Copy constructor
    /// @param other the vector to copy
    Vector(const Vector& other) { Copy(other.impl_.slice); }

    /// Move constructor
    /// @param other the vector to move
    Vector(Vector&& other) { Move(std::move(other)); }

    /// Copy constructor (differing N length)
    /// @param other the vector to copy
    template <size_t N2>
    Vector(const Vector<T, N2>& other) {
        Copy(other.impl_.slice);
    }

    /// Move constructor (differing N length)
    /// @param other the vector to move
    template <size_t N2>
    Vector(Vector<T, N2>&& other) {
        Move(std::move(other));
    }

    /// Copy constructor with covariance / const conversion
    /// @param other the vector to copy
    /// @see CanReinterpretSlice for rules about conversion
    template <typename U,
              size_t N2,
              ReinterpretMode MODE,
              typename = std::enable_if_t<CanReinterpretSlice<MODE, T, U>>>
    Vector(const Vector<U, N2>& other) {  // NOLINT(runtime/explicit)
        Copy(other.impl_.slice.template Reinterpret<T, MODE>);
    }

    /// Move constructor with covariance / const conversion
    /// @param other the vector to move
    /// @see CanReinterpretSlice for rules about conversion
    template <typename U,
              size_t N2,
              ReinterpretMode MODE,
              typename = std::enable_if_t<CanReinterpretSlice<MODE, T, U>>>
    Vector(Vector<U, N2>&& other) {  // NOLINT(runtime/explicit)
        Move(std::move(other));
    }

    /// Move constructor from a mutable vector reference
    /// @param other the vector reference to move
    Vector(VectorRef<T>&& other) { MoveOrCopy(std::move(other)); }  // NOLINT(runtime/explicit)

    /// Copy constructor from an immutable vector reference
    /// @param other the vector reference to copy
    Vector(const VectorRef<T>& other) { Copy(other.slice_); }  // NOLINT(runtime/explicit)

    /// Copy constructor from an immutable slice
    /// @param other the slice to copy
    Vector(const Slice<T>& other) {  // NOLINT(runtime/explicit)
        Copy(other);
    }

    /// Copy constructor from an immutable slice
    /// @param other the slice to copy
    /// @note This overload only exists to keep MSVC happy. The compiler should be able to match
    /// `Slice<U>`.
    Vector(const Slice<const T>& other) {  // NOLINT(runtime/explicit)
        Copy(other);
    }

    /// Copy constructor from an immutable slice
    /// @param other the slice to copy
    template <typename U>
    Vector(const Slice<U>& other) {  // NOLINT(runtime/explicit)
        Copy(other);
    }

    /// Destructor
    ~Vector() { ClearAndFree(); }

    /// Assignment operator
    /// @param other the vector to copy
    /// @returns this vector so calls can be chained
    Vector& operator=(const Vector& other) {
        if (&other != this) {
            Copy(other.impl_.slice);
        }
        return *this;
    }

    /// Move operator
    /// @param other the vector to move
    /// @returns this vector so calls can be chained
    Vector& operator=(Vector&& other) {
        if (&other != this) {
            Move(std::move(other));
        }
        return *this;
    }

    /// Assignment operator (differing N length)
    /// @param other the vector to copy
    /// @returns this vector so calls can be chained
    template <size_t N2>
    Vector& operator=(const Vector<T, N2>& other) {
        Copy(other.impl_.slice);
        return *this;
    }

    /// Move operator (differing N length)
    /// @param other the vector to copy
    /// @returns this vector so calls can be chained
    template <size_t N2>
    Vector& operator=(Vector<T, N2>&& other) {
        Move(std::move(other));
        return *this;
    }

    /// Assignment operator (differing N length)
    /// @param other the vector reference to copy
    /// @returns this vector so calls can be chained
    Vector& operator=(const VectorRef<T>& other) {
        if (&other.slice_ != &impl_.slice) {
            Copy(other.slice_);
        }
        return *this;
    }

    /// Move operator (differing N length)
    /// @param other the vector reference to copy
    /// @returns this vector so calls can be chained
    Vector& operator=(VectorRef<T>&& other) {
        if (&other.slice_ != &impl_.slice) {
            MoveOrCopy(std::move(other));
        }
        return *this;
    }

    /// Assignment operator for Slice
    /// @param other the slice to copy
    /// @returns this vector so calls can be chained
    Vector& operator=(const Slice<T>& other) {
        Copy(other);
        return *this;
    }

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    T& operator[](size_t i) { return impl_.slice[i]; }

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    const T& operator[](size_t i) const { return impl_.slice[i]; }

    /// @return the number of elements in the vector
    size_t Length() const { return impl_.slice.len; }

    /// @return the number of elements that the vector could hold before a heap allocation needs to
    /// be made
    size_t Capacity() const { return impl_.slice.cap; }

    /// Reserves memory to hold at least `new_cap` elements
    /// @param new_cap the new vector capacity
    void Reserve(size_t new_cap) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        if (new_cap > impl_.slice.cap) {
            auto* old_data = impl_.slice.data;
            impl_.Allocate(new_cap);
            for (size_t i = 0; i < impl_.slice.len; i++) {
                new (&impl_.slice.data[i]) T(std::move(old_data[i]));
                old_data[i].~T();
            }
            impl_.Free(old_data);
        }
    }

    /// Resizes the vector to the given length, expanding capacity if necessary.
    /// New elements are zero-initialized
    /// @param new_len the new vector length
    void Resize(size_t new_len) {
        Reserve(new_len);
        for (size_t i = impl_.slice.len; i > new_len; i--) {  // Shrink
            impl_.slice.data[i - 1].~T();
        }
        for (size_t i = impl_.slice.len; i < new_len; i++) {  // Grow
            new (&impl_.slice.data[i]) T{};
        }
        impl_.slice.len = new_len;
    }

    /// Resizes the vector to the given length, expanding capacity if necessary.
    /// @param new_len the new vector length
    /// @param value the value to copy into the new elements
    void Resize(size_t new_len, const T& value) {
        Reserve(new_len);
        for (size_t i = impl_.slice.len; i > new_len; i--) {  // Shrink
            impl_.slice.data[i - 1].~T();
        }
        for (size_t i = impl_.slice.len; i < new_len; i++) {  // Grow
            new (&impl_.slice.data[i]) T{value};
        }
        impl_.slice.len = new_len;
    }

    /// Copies all the elements from `other` to this vector, replacing the content of this vector.
    /// @param other the
    template <typename T2, size_t N2>
    void Copy(const Vector<T2, N2>& other) {
        Copy(other.impl_.slice);
    }

    /// Clears all elements from the vector, keeping the capacity the same.
    void Clear() {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        for (size_t i = 0; i < impl_.slice.len; i++) {
            impl_.slice.data[i].~T();
        }
        impl_.slice.len = 0;
    }

    /// Appends a new element to the vector.
    /// @param el the element to copy to the vector.
    void Push(const T& el) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        if (impl_.slice.len >= impl_.slice.cap) {
            Grow();
        }
        new (&impl_.slice.data[impl_.slice.len++]) T(el);
    }

    /// Appends a new element to the vector.
    /// @param el the element to move to the vector.
    void Push(T&& el) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        if (impl_.slice.len >= impl_.slice.cap) {
            Grow();
        }
        new (&impl_.slice.data[impl_.slice.len++]) T(std::move(el));
    }

    /// Appends a new element to the vector.
    /// @param args the arguments to pass to the element constructor.
    template <typename... ARGS>
    void Emplace(ARGS&&... args) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        if (impl_.slice.len >= impl_.slice.cap) {
            Grow();
        }
        new (&impl_.slice.data[impl_.slice.len++]) T{std::forward<ARGS>(args)...};
    }

    /// Removes and returns the last element from the vector.
    /// @returns the popped element
    T Pop() {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        TINT_ASSERT(!IsEmpty());
        auto& el = impl_.slice.data[--impl_.slice.len];
        auto val = std::move(el);
        el.~T();
        return val;
    }

    /// Inserts the element @p element before the element at @p before
    /// @param before the index of the element to insert before
    /// @param element the element to insert
    template <typename EL>
    void Insert(size_t before, EL&& element) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        TINT_ASSERT(before <= Length());
        size_t n = Length();
        Resize(Length() + 1);
        // Shuffle
        for (size_t i = n; i > before; i--) {
            auto& src = impl_.slice.data[i - 1];
            auto& dst = impl_.slice.data[i];
            dst = std::move(src);
        }
        // Insert
        impl_.slice.data[before] = std::forward<EL>(element);
    }

    /// Removes @p count elements from the vector
    /// @param start the index of the first element to remove
    /// @param count the number of elements to remove
    void Erase(size_t start, size_t count = 1) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        TINT_ASSERT(start < Length());
        TINT_ASSERT((start + count) <= Length());
        // Shuffle
        for (size_t i = start + count; i < impl_.slice.len; i++) {
            auto& src = impl_.slice.data[i];
            auto& dst = impl_.slice.data[i - count];
            dst = std::move(src);
        }
        // Pop
        for (size_t i = 0; i < count; i++) {
            auto& el = impl_.slice.data[--impl_.slice.len];
            el.~T();
        }
    }

    /// Removes all the elements from the vector that match the predicate function.
    /// @param predicate the predicate function with the signature `bool(const T&)`. This function
    /// should return `true` for elements that should be removed from the vector.
    template <typename PREDICATE>
    void EraseIf(PREDICATE&& predicate) {
        TINT_VECTOR_MUTATION_CHECK_ASSERT(iterator_count_ == 0);
        // Shuffle
        size_t num_removed = 0;
        for (size_t i = 0; i < impl_.slice.len; i++) {
            auto& el = impl_.slice.data[i];
            bool remove = predicate(const_cast<const T&>(el));
            if (num_removed > 0) {
                auto& dst = impl_.slice.data[i - num_removed];
                dst = std::move(el);
            }
            if (remove) {
                num_removed++;
            }
        }
        // Pop
        for (size_t i = 0; i < num_removed; i++) {
            auto& el = impl_.slice.data[--impl_.slice.len];
            el.~T();
        }
    }

    /// Sort sorts the vector in-place using the predicate function @p pred
    /// @param pred a function that has the signature `bool(const T& a, const T& b)` which returns
    /// true if `a` is ordered before `b`.
    template <typename PREDICATE>
    void Sort(PREDICATE&& pred) {
        std::sort(begin(), end(), std::forward<PREDICATE>(pred));
    }

    /// Sort sorts the vector in-place using `T::operator<()`
    /// @returns this vector so calls can be chained
    Vector& Sort() {
        Sort([](auto& a, auto& b) { return a < b; });
        return *this;
    }

    /// Reverse reversed the vector in-place
    void Reverse() {
        size_t n = Length();
        size_t mid = n / 2;
        auto& self = *this;
        for (size_t i = 0; i < mid; i++) {
            std::swap(self[i], self[n - i - 1]);
        }
    }

    /// @returns true if the predicate function returns true for any of the elements of the vector
    /// @param pred a function-like with the signature `bool(T)`
    template <typename PREDICATE>
    bool Any(PREDICATE&& pred) const {
        return std::any_of(begin(), end(), std::forward<PREDICATE>(pred));
    }

    /// @returns false if the predicate function returns false for any of the elements of the vector
    /// @param pred a function-like with the signature `bool(T)`
    template <typename PREDICATE>
    bool All(PREDICATE&& pred) const {
        return std::all_of(begin(), end(), std::forward<PREDICATE>(pred));
    }

    /// @returns true if the vector is empty.
    bool IsEmpty() const { return impl_.slice.len == 0; }

    /// @returns a reference to the first element in the vector
    T& Front() { return impl_.slice.Front(); }

    /// @returns a reference to the first element in the vector
    const T& Front() const { return impl_.slice.Front(); }

    /// @returns a reference to the last element in the vector
    T& Back() { return impl_.slice.Back(); }

    /// @returns a reference to the last element in the vector
    const T& Back() const { return impl_.slice.Back(); }

#if TINT_VECTOR_MUTATION_CHECKS_ENABLED
    /// @returns a forward iterator to the first element of the vector
    iterator begin() { return iterator{impl_.slice.begin(), &iterator_count_}; }

    /// @returns a forward iterator to the first element of the vector
    const const_iterator begin() const {
        return const_iterator{impl_.slice.begin(), &iterator_count_};
    }

    /// @returns a forward iterator to one-pass the last element of the vector
    iterator end() { return iterator{impl_.slice.end(), &iterator_count_}; }

    /// @returns a forward iterator to one-pass the last element of the vector
    const const_iterator end() const { return const_iterator{impl_.slice.end(), &iterator_count_}; }

    /// @returns a reverse iterator to the last element of the vector
    reverse_iterator rbegin() { return reverse_iterator{impl_.slice.end(), &iterator_count_} + 1; }

    /// @returns a reverse iterator to the last element of the vector
    const const_reverse_iterator rbegin() const {
        return const_reverse_iterator{impl_.slice.end(), &iterator_count_} + 1;
    }

    /// @returns a reverse iterator to one element before the first element of the vector
    reverse_iterator rend() { return reverse_iterator{impl_.slice.begin(), &iterator_count_} + 1; }

    /// @returns a reverse iterator to one element before the first element of the vector
    const const_reverse_iterator rend() const {
        return const_reverse_iterator{impl_.slice.begin(), &iterator_count_} + 1;
    }
#else
    /// @returns a forward iterator to the first element of the vector
    iterator begin() { return iterator{impl_.slice.begin()}; }

    /// @returns a forward iterator to the first element of the vector
    const const_iterator begin() const { return const_iterator{impl_.slice.begin()}; }

    /// @returns a forward iterator to one-pass the last element of the vector
    iterator end() { return iterator{impl_.slice.end()}; }

    /// @returns a forward iterator to one-pass the last element of the vector
    const const_iterator end() const { return const_iterator{impl_.slice.end()}; }

    /// @returns a reverse iterator to the last element of the vector
    reverse_iterator rbegin() { return reverse_iterator{impl_.slice.end()} + 1; }

    /// @returns a reverse iterator to the last element of the vector
    const const_reverse_iterator rbegin() const {
        return const_reverse_iterator{impl_.slice.end()} + 1;
    }

    /// @returns a reverse iterator to one element before the first element of the vector
    reverse_iterator rend() { return reverse_iterator{impl_.slice.begin()} + 1; }

    /// @returns a reverse iterator to one element before the first element of the vector
    const const_reverse_iterator rend() const {
        return const_reverse_iterator{impl_.slice.begin()} + 1;
    }
#endif

    /// @returns a hash code for this Vector
    tint::HashCode HashCode() const {
        auto hash = Hash(Length());
        for (auto& el : *this) {
            hash = HashCombine(hash, el);
        }
        return hash;
    }

    /// Equality operator
    /// @param other the other vector
    /// @returns true if this vector is the same length as `other`, and all elements are equal.
    template <typename T2, size_t N2>
    bool operator==(const Vector<T2, N2>& other) const {
        const size_t len = Length();
        if (len != other.Length()) {
            return false;
        }
        for (size_t i = 0; i < len; i++) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    /// Inequality operator
    /// @param other the other vector
    /// @returns true if this vector is not the same length as `other`, or all elements are not
    ///          equal.
    template <typename T2, size_t N2>
    bool operator!=(const Vector<T2, N2>& other) const {
        return !(*this == other);
    }

    /// @returns the internal slice of the vector
    tint::Slice<T> Slice() { return impl_.slice; }

    /// @returns the internal slice of the vector
    tint::Slice<const T> Slice() const { return impl_.slice; }

  private:
    /// Friend class (differing specializations of this class)
    template <typename, size_t>
    friend class Vector;

    /// Friend class
    template <typename>
    friend class VectorRef;

    /// Friend class
    template <typename>
    friend class VectorRef;

    template <typename... Ts>
    void AppendVariadic(Ts&&... args) {
        ((new (&impl_.slice.data[impl_.slice.len++]) T(std::forward<Ts>(args))), ...);
    }

    /// Expands the capacity of the vector
    void Grow() { Reserve(std::max(impl_.slice.cap, static_cast<size_t>(1)) * 2); }

    /// Moves 'other' to this vector, if possible, otherwise performs a copy.
    void MoveOrCopy(VectorRef<T>&& other) {
        if (other.can_move_) {
            // Just steal the slice.
            ClearAndFree();
            impl_.slice = other.slice_;
            other.slice_ = {};
        } else {
            Copy(other.slice_);
        }
    }

    /// Copies all the elements from `other` to this vector, replacing the content of this vector.
    /// @param other the slice to copy
    template <typename U>
    void Copy(const tint::Slice<U>& other) {
        if (impl_.slice.cap < other.len) {
            ClearAndFree();
            impl_.Allocate(other.len);
        } else {
            Clear();
        }

        impl_.slice.len = other.len;
        for (size_t i = 0; i < impl_.slice.len; i++) {
            new (&impl_.slice.data[i]) T{other.data[i]};
        }
    }

    /// Moves all the elements from `other` to this vector, replacing the content of this vector.
    /// @param other the vector to move
    template <typename U, size_t N2>
    void Move(Vector<U, N2>&& other) {
        auto& other_slice = other.impl_.slice;
        if constexpr (std::is_same_v<T, U>) {
            if (other.impl_.CanMove()) {
                // Just steal the slice.
                ClearAndFree();
                impl_.slice = other_slice;
                other_slice = {};
                return;
            }
        }

        // Can't steal the slice, so we have to move the elements instead.

        // Ensure we have capacity for all the elements
        if (impl_.slice.cap < other_slice.len) {
            ClearAndFree();
            impl_.Allocate(other_slice.len);
        } else {
            Clear();
        }

        // Move each of the elements.
        impl_.slice.len = other_slice.len;
        for (size_t i = 0; i < impl_.slice.len; i++) {
            new (&impl_.slice.data[i]) T{std::move(other_slice.data[i])};
        }

        // Clear other
        other.Clear();
    }

    /// Clears the vector, then frees the slice data.
    void ClearAndFree() {
        Clear();
        impl_.Free(impl_.slice.data);
    }

    /// True if this vector uses a small array for small object optimization.
    constexpr static bool HasSmallArray = N > 0;

    /// A structure that has the same size and alignment as T.
    using TStorage = AlignedStorage<T>;

    /// The internal structure for the vector with a small array.
    struct ImplWithSmallArray {
        TStorage small_arr[N];
        tint::Slice<T> slice = {&small_arr[0].Get(), 0, N};

        /// Allocates a new vector of `T` either from #small_arr, or from the heap, then assigns the
        /// pointer it to #slice.data, and updates #slice.cap.
        void Allocate(size_t new_cap) {
            if (new_cap < N) {
                slice.data = &small_arr[0].Get();
                slice.cap = N;
            } else {
                slice.data = Bitcast<T*>(new TStorage[new_cap]);
                slice.cap = new_cap;
            }
        }

        /// Frees `data`, if isn't a pointer to #small_arr
        void Free(T* data) const {
            if (data != &small_arr[0].Get()) {
                delete[] Bitcast<TStorage*>(data);
            }
        }

        /// Indicates whether the slice structure can be std::move()d.
        /// @returns true if #slice.data does not point to #small_arr
        bool CanMove() const { return slice.data != &small_arr[0].Get(); }
    };

    /// The internal structure for the vector without a small array.
    struct ImplWithoutSmallArray {
        tint::Slice<T> slice = Empty;

        /// Allocates a new vector of `T` and assigns it to #slice.data, and updates #slice.cap.
        void Allocate(size_t new_cap) {
            slice.data = Bitcast<T*>(new TStorage[new_cap]);
            slice.cap = new_cap;
        }

        /// Frees `data`.
        void Free(T* data) const { delete[] Bitcast<TStorage*>(data); }

        /// Indicates whether the slice structure can be std::move()d.
        /// @returns true
        bool CanMove() const { return true; }
    };

    /// Either a ImplWithSmallArray or ImplWithoutSmallArray based on N.
    std::conditional_t<HasSmallArray, ImplWithSmallArray, ImplWithoutSmallArray> impl_;

    /// The current number of iterators referring to this vector
    mutable std::atomic<uint32_t> iterator_count_ = 0;
};

namespace detail {

/// Helper for determining the Vector element type (`T`) from the vector's constuctor arguments
/// @tparam IS_CASTABLE true if the types of `Ts` derive from CastableBase
/// @tparam Ts the vector constructor argument types to infer the vector element type from.
template <bool IS_CASTABLE, typename... Ts>
struct VectorCommonType;

/// VectorCommonType specialization for non-castable types.
template <typename... Ts>
struct VectorCommonType</*IS_CASTABLE*/ false, Ts...> {
    /// The common T type to use for the vector
    using type = std::common_type_t<Ts...>;
};

/// VectorCommonType specialization for castable types.
template <typename... Ts>
struct VectorCommonType</*IS_CASTABLE*/ true, Ts...> {
    /// The common Castable type (excluding pointer)
    using common_ty = CastableCommonBase<std::remove_pointer_t<Ts>...>;
    /// The common T type to use for the vector
    using type = std::conditional_t<(std::is_const_v<std::remove_pointer_t<Ts>> || ...),
                                    const common_ty*,
                                    common_ty*>;
};

}  // namespace detail

/// Helper for determining the Vector element type (`T`) from the vector's constuctor arguments
template <typename... Ts>
using VectorCommonType =
    typename tint::detail::VectorCommonType<IsCastable<std::remove_pointer_t<Ts>...>, Ts...>::type;

/// Deduction guide for Vector
template <typename... Ts>
Vector(Ts...) -> Vector<VectorCommonType<Ts...>, sizeof...(Ts)>;

/// VectorRef is a weak reference to a Vector, used to pass vectors as parameters, avoiding copies
/// between the caller and the callee, or as an non-static sized accessor on a vector. VectorRef can
/// accept a Vector of any 'N' value, decoupling the caller's vector internal size from the callee's
/// vector size. A VectorRef tracks the usage of moves either side of the call. If at the call site,
/// a Vector argument is moved to a VectorRef parameter, and within the callee, the VectorRef
/// parameter is moved to a Vector, then the Vector heap allocation will be moved. For example:
///
/// ```
///     void func_a() {
///        Vector<std::string, 4> vec;
///        // logic to populate 'vec'.
///        func_b(std::move(vec)); // Constructs a VectorRef tracking the move here.
///     }
///
///     void func_b(VectorRef<std::string> vec_ref) {
///        // A move was made when calling func_b, so the vector can be moved instead of copied.
///        Vector<std::string, 2> vec(std::move(vec_ref));
///     }
/// ```
///
/// Aside from this move pattern, a VectorRef provides an immutable reference to the Vector.
template <typename T>
class VectorRef {
    /// @returns an empty slice.
    static tint::Slice<T>& EmptySlice() {
        static tint::Slice<T> empty;
        return empty;
    }

  public:
    /// Type of `T`.
    using value_type = T;

    /// Constructor - empty reference
    VectorRef() : slice_(EmptySlice()) {}

    /// Constructor
    VectorRef(EmptyType) : slice_(EmptySlice()) {}  // NOLINT(runtime/explicit)

    /// Constructor from a Slice
    /// @param slice the slice
    VectorRef(tint::Slice<T>& slice)  // NOLINT(runtime/explicit)
        : slice_(slice) {}

    /// Constructor from a Vector
    /// @param vector the vector to create a reference of
    template <size_t N>
    VectorRef(Vector<T, N>& vector)  // NOLINT(runtime/explicit)
        : slice_(vector.impl_.slice) {}

    /// Constructor from a const Vector
    /// @param vector the vector to create a reference of
    template <size_t N>
    VectorRef(const Vector<T, N>& vector)  // NOLINT(runtime/explicit)
        : slice_(const_cast<tint::Slice<T>&>(vector.impl_.slice)) {}

    /// Constructor from a moved Vector
    /// @param vector the vector being moved
    template <size_t N>
    VectorRef(Vector<T, N>&& vector)  // NOLINT(runtime/explicit)
        : slice_(vector.impl_.slice), can_move_(vector.impl_.CanMove()) {}

    /// Copy constructor
    /// @param other the vector reference
    VectorRef(const VectorRef& other) : slice_(other.slice_) {}

    /// Move constructor
    /// @param other the vector reference
    VectorRef(VectorRef&& other) = default;

    /// Copy constructor with covariance / const conversion
    /// @param other the other vector reference
    template <typename U,
              typename = std::enable_if_t<CanReinterpretSlice<ReinterpretMode::kSafe, T, U>>>
    VectorRef(const VectorRef<U>& other)  // NOLINT(runtime/explicit)
        : slice_(other.slice_.template Reinterpret<T>()) {}

    /// Move constructor with covariance / const conversion
    /// @param other the vector reference
    template <typename U,
              typename = std::enable_if_t<CanReinterpretSlice<ReinterpretMode::kSafe, T, U>>>
    VectorRef(VectorRef<U>&& other)  // NOLINT(runtime/explicit)
        : slice_(other.slice_.template Reinterpret<T>()), can_move_(other.can_move_) {}

    /// Constructor from a Vector with covariance / const conversion
    /// @param vector the vector to create a reference of
    /// @see CanReinterpretSlice for rules about conversion
    template <typename U,
              size_t N,
              typename = std::enable_if_t<CanReinterpretSlice<ReinterpretMode::kSafe, T, U>>>
    VectorRef(const Vector<U, N>& vector)  // NOLINT(runtime/explicit)
        : slice_(const_cast<tint::Slice<U>&>(vector.impl_.slice).template Reinterpret<T>()) {}

    /// Constructor from a moved Vector with covariance / const conversion
    /// @param vector the vector to create a reference of
    /// @see CanReinterpretSlice for rules about conversion
    template <typename U,
              size_t N,
              typename = std::enable_if_t<CanReinterpretSlice<ReinterpretMode::kSafe, T, U>>>
    VectorRef(Vector<U, N>&& vector)  // NOLINT(runtime/explicit)
        : slice_(vector.impl_.slice.template Reinterpret<T>()), can_move_(vector.impl_.CanMove()) {}

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    const T& operator[](size_t i) const { return slice_[i]; }

    /// @return the number of elements in the vector
    size_t Length() const { return slice_.len; }

    /// @return the number of elements that the vector could hold before a heap allocation needs to
    /// be made
    size_t Capacity() const { return slice_.cap; }

    /// @return a reinterpretation of this VectorRef as elements of type U.
    /// @note this is doing a reinterpret_cast of elements. It is up to the caller to ensure that
    /// this is a safe operation.
    template <typename U>
    VectorRef<U> ReinterpretCast() const {
        return {slice_.template Reinterpret<U, ReinterpretMode::kUnsafe>()};
    }

    /// @returns the internal slice of the vector
    tint::Slice<T> Slice() { return slice_; }

    /// @returns true if the vector is empty.
    bool IsEmpty() const { return slice_.len == 0; }

    /// @returns a reference to the first element in the vector
    const T& Front() const { return slice_.Front(); }

    /// @returns a reference to the last element in the vector
    const T& Back() const { return slice_.Back(); }

    /// @returns a pointer to the first element in the vector
    const T* begin() const { return slice_.begin(); }

    /// @returns a pointer to one past the last element in the vector
    const T* end() const { return slice_.end(); }

    /// @returns a reverse iterator starting with the last element in the vector
    auto rbegin() const { return slice_.rbegin(); }

    /// @returns the end for a reverse iterator
    auto rend() const { return slice_.rend(); }

    /// @returns a hash code of the Vector
    tint::HashCode HashCode() const {
        auto hash = Hash(Length());
        for (auto& el : *this) {
            hash = HashCombine(hash, el);
        }
        return hash;
    }

    /// @returns true if the predicate function returns true for any of the elements of the vector
    /// @param pred a function-like with the signature `bool(T)`
    template <typename PREDICATE>
    bool Any(PREDICATE&& pred) const {
        return std::any_of(begin(), end(), std::forward<PREDICATE>(pred));
    }

  private:
    /// Friend class
    template <typename, size_t>
    friend class Vector;

    /// Friend class
    template <typename>
    friend class VectorRef;

    /// Friend class
    template <typename>
    friend class VectorRef;

    /// The slice of the vector being referenced.
    tint::Slice<T>& slice_;
    /// Whether the slice data is passed by r-value reference, and can be moved.
    bool can_move_ = false;
};

/// Helper for converting a Vector to a std::vector.
/// @param vector the input vector
/// @return the converted vector
/// @note This helper exists to help code migration. Avoid if possible.
template <typename T, size_t N>
std::vector<T> ToStdVector(const Vector<T, N>& vector) {
    std::vector<T> out;
    out.reserve(vector.Length());
    for (auto& el : vector) {
        out.emplace_back(el);
    }
    return out;
}

/// Helper for constructing a Vector from a Slice. Only the size must be supplied as the type is
/// deduced.
/// @param slice the input slice
/// @return the converted vector
/// @note This helper is useful because Vectors require a size parameter, but because it is the
/// second template parameter to a Vector, both the type and size parameters must be explicitly
/// declared. Furthermore, Slices are often of const pointer/reference type, but a Vector cannot be
/// of const pointer/reference type, again requiring the caller to be explicit. This helper makes it
/// possible to only specify the size.
template <size_t N, typename T>
auto ToVector(const tint::Slice<T>& slice) {
    // If Slice is of type 'T* const', make it 'T*' (or 'T& const', make it 'T&') as Vectors cannot
    // be of const pointer/reference type.
    using U = std::conditional_t<std::is_pointer_v<T> || std::is_reference_v<T>,
                                 std::remove_const_t<T>, T>;
    return Vector<U, N>{slice};
}

/// Helper for converting a std::vector to a Vector.
/// @param vector the input vector
/// @return the converted vector
/// @note This helper exists to help code migration. Avoid if possible.
template <typename T, size_t N = 0>
Vector<T, N> ToVector(const std::vector<T>& vector) {
    Vector<T, N> out;
    out.Reserve(vector.size());
    for (auto& el : vector) {
        out.Push(el);
    }
    return out;
}

/// Prints the vector @p vec to @p o
/// @param o the stream to write to
/// @param vec the vector
/// @return the stream so calls can be chained
template <typename STREAM, typename T, size_t N, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& o, const Vector<T, N>& vec) {
    o << "[";
    bool first = true;
    for (auto& el : vec) {
        if (!first) {
            o << ", ";
        }
        first = false;
        o << el;
    }
    o << "]";
    return o;
}

/// Prints the vector @p vec to @p o
/// @param o the stream to write to
/// @param vec the vector reference
/// @return the stream so calls can be chained
template <typename STREAM, typename T, typename = traits::EnableIfIsOStream<STREAM>>
auto& operator<<(STREAM& o, VectorRef<T> vec) {
    o << "[";
    bool first = true;
    for (auto& el : vec) {
        if (!first) {
            o << ", ";
        }
        first = false;
        o << el;
    }
    o << "]";
    return o;
}

namespace detail {

/// IsVectorLike<T>::value is true if T is a Vector or VectorRef.
template <typename T>
struct IsVectorLike {
    /// Non-specialized form of IsVectorLike defaults to false
    static constexpr bool value = false;
};

/// IsVectorLike specialization for Vector
template <typename T, size_t N>
struct IsVectorLike<Vector<T, N>> {
    /// True for the IsVectorLike specialization of Vector
    static constexpr bool value = true;
};

/// IsVectorLike specialization for VectorRef
template <typename T>
struct IsVectorLike<VectorRef<T>> {
    /// True for the IsVectorLike specialization of VectorRef
    static constexpr bool value = true;
};
}  // namespace detail

/// True if T is a Vector<T, N> or VectorRef<T>
template <typename T>
static constexpr bool IsVectorLike = tint::detail::IsVectorLike<T>::value;

}  // namespace tint

TINT_END_DISABLE_WARNING(UNSAFE_BUFFER_USAGE);

#endif  // SRC_TINT_UTILS_CONTAINERS_VECTOR_H_
