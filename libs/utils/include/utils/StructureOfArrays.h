/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef TNT_UTILS_STRUCTUREOFARRAYS_H
#define TNT_UTILS_STRUCTUREOFARRAYS_H

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/Slice.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <array>        // note: this is safe, see how std::array is used below (inline / private)
#include <cstddef>
#include <iterator>     // for std::random_access_iterator_tag
#include <tuple>
#include <utility>

namespace utils {

template <typename Allocator, typename ... Elements>
class StructureOfArraysBase {
    // number of elements
    static constexpr const size_t kArrayCount = sizeof...(Elements);

public:
    using SoA = StructureOfArraysBase<Allocator, Elements...>;

    using Structure = std::tuple<Elements...>;

    // Type of the Nth array
    template<size_t N>
    using TypeAt = typename std::tuple_element_t<N, Structure>;

    // Number of arrays
    static constexpr size_t getArrayCount() noexcept { return kArrayCount; }

    // Size needed to store "size" array elements
    static size_t getNeededSize(size_t size) noexcept {
        return getOffset(kArrayCount - 1, size) + sizeof(TypeAt<kArrayCount - 1>) * size;
    }

    // --------------------------------------------------------------------------------------------

    class IteratorValue;
    template<typename T> class Iterator;
    using iterator = Iterator<StructureOfArraysBase*>;
    using const_iterator = Iterator<StructureOfArraysBase const*>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    /*
     * An object that represents a reference to the type dereferenced by iterator.
     * In other words, it's the return type of iterator::operator*(), and since it
     * cannot be a C++ reference (&), it's an object that acts like it.
     */
    class IteratorValueRef {
        friend class IteratorValue;
        friend iterator;
        friend const_iterator;
        StructureOfArraysBase* const UTILS_RESTRICT soa;
        size_t const index;

        IteratorValueRef(StructureOfArraysBase* soa, size_t index) : soa(soa), index(index) { }

        // assigns a value_type to a reference (i.e. assigns to what's pointed to by the reference)
        template<size_t ... Is>
        IteratorValueRef& assign(IteratorValue const& rhs, std::index_sequence<Is...>);

        // assigns a value_type to a reference (i.e. assigns to what's pointed to by the reference)
        template<size_t ... Is>
        IteratorValueRef& assign(IteratorValue&& rhs, std::index_sequence<Is...>) noexcept;

        // objects pointed to by reference can be swapped, so provide the special swap() function.
        friend void swap(IteratorValueRef lhs, IteratorValueRef rhs) {
            lhs.soa->swap(lhs.index, rhs.index);
        }

    public:
        // references can be created by copy-assignment only
        IteratorValueRef(IteratorValueRef const& rhs) noexcept : soa(rhs.soa), index(rhs.index) { }

        // copy the content of a reference to the content of this one
        IteratorValueRef& operator=(IteratorValueRef const& rhs);

        // move the content of a reference to the content of this one
        IteratorValueRef& operator=(IteratorValueRef&& rhs) noexcept;

        // copy a value_type to the content of this reference
        IteratorValueRef& operator=(IteratorValue const& rhs) {
            return assign(rhs, std::make_index_sequence<kArrayCount>());
        }

        // move a value_type to the content of this reference
        IteratorValueRef& operator=(IteratorValue&& rhs) noexcept {
            return assign(rhs, std::make_index_sequence<kArrayCount>());
        }

        // access the elements of this reference (i.e. the "fields" of the structure)
        template<size_t I> TypeAt<I> const& get() const { return soa->elementAt<I>(index); }
        template<size_t I> TypeAt<I>& get() { return soa->elementAt<I>(index); }
    };


    /*
     * The value_type of iterator. This is basically the "structure" of the SoA.
     * Internally we're using a tuple<> to store the data.
     * This object is not trivial to construct, as it copies an entry of the SoA.
     */
    class IteratorValue {
        friend class IteratorValueRef;
        friend iterator;
        friend const_iterator;
        using Type = std::tuple<typename std::decay<Elements>::type...>;
        Type elements;

        template<size_t ... Is>
        static Type init(IteratorValueRef const& rhs, std::index_sequence<Is...>) {
            return Type{ rhs.soa->template elementAt<Is>(rhs.index)... };
        }

        template<size_t ... Is>
        static Type init(IteratorValueRef&& rhs, std::index_sequence<Is...>) noexcept {
            return Type{ std::move(rhs.soa->template elementAt<Is>(rhs.index))... };
        }

    public:
        IteratorValue(IteratorValue const& rhs) = default;
        IteratorValue(IteratorValue&& rhs) noexcept = default;
        IteratorValue& operator=(IteratorValue const& rhs) = default;
        IteratorValue& operator=(IteratorValue&& rhs) noexcept = default;

        // initialize and assign from a StructureRef
        IteratorValue(IteratorValueRef const& rhs)
                : elements(init(rhs, std::make_index_sequence<kArrayCount>())) {}
        IteratorValue(IteratorValueRef&& rhs) noexcept
                : elements(init(rhs, std::make_index_sequence<kArrayCount>())) {}
        IteratorValue& operator=(IteratorValueRef const& rhs) { return operator=(IteratorValue(rhs)); }
        IteratorValue& operator=(IteratorValueRef&& rhs) noexcept { return operator=(IteratorValue(rhs)); }

        // access the elements of this value_Type (i.e. the "fields" of the structure)
        template<size_t I> TypeAt<I> const& get() const { return std::get<I>(elements); }
        template<size_t I> TypeAt<I>& get() { return std::get<I>(elements); }
    };


    /*
     * An iterator to the SoA. This is only intended to be used with STL's algorithm, e.g.: sort().
     * Normally, SoA is not iterated globally, but rather an array at a time.
     * Iterating itself is not too costly, as well as dereferencing by reference. However,
     * dereferencing by value is.
     */
    template<typename CVQualifiedSOAPointer>
    class Iterator {
        friend class StructureOfArraysBase;
        CVQualifiedSOAPointer soa; // don't use restrict, can have aliases if multiple iterators are created
        size_t index;

        Iterator(CVQualifiedSOAPointer soa, size_t index) : soa(soa), index(index) {}

    public:
        using value_type = IteratorValue;
        using reference = IteratorValueRef;
        using pointer = IteratorValueRef*;    // FIXME: this should be a StructurePtr type
        using difference_type = ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;

        Iterator(Iterator const& rhs) noexcept = default;
        Iterator& operator=(Iterator const& rhs) = default;

        reference operator*() const { return { soa, index }; }
        reference operator*() { return { soa, index }; }
        reference operator[](size_t n) { return *(*this + n); }

        template<size_t I> TypeAt<I> const& get() const { return soa->template elementAt<I>(index); }
        template<size_t I> TypeAt<I>& get() { return soa->template elementAt<I>(index); }

        Iterator& operator++() { ++index; return *this; }
        Iterator& operator--() { --index; return *this; }
        Iterator& operator+=(size_t n) { index += n; return *this; }
        Iterator& operator-=(size_t n) { index -= n; return *this; }
        Iterator operator+(size_t n) const { return { soa, index + n }; }
        Iterator operator-(size_t n) const { return { soa, index - n }; }
        difference_type operator-(Iterator const& rhs) const { return index - rhs.index; }
        bool operator==(Iterator const& rhs) const { return (index == rhs.index); }
        bool operator!=(Iterator const& rhs) const { return (index != rhs.index); }
        bool operator>=(Iterator const& rhs) const { return (index >= rhs.index); }
        bool operator> (Iterator const& rhs) const { return (index >  rhs.index); }
        bool operator<=(Iterator const& rhs) const { return (index <= rhs.index); }
        bool operator< (Iterator const& rhs) const { return (index <  rhs.index); }

        // Postfix operator needed by Microsoft STL.
        const Iterator operator++(int) { Iterator it(*this); index++; return it; }
        const Iterator operator--(int) { Iterator it(*this); index--; return it; }
    };

    iterator begin() noexcept { return { this, 0u }; }
    iterator end() noexcept { return { this, mSize }; }
    const_iterator begin() const noexcept { return { this, 0u }; }
    const_iterator end() const noexcept { return { this, mSize }; }

    // --------------------------------------------------------------------------------------------

    StructureOfArraysBase() = default;

    explicit StructureOfArraysBase(size_t capacity) {
        setCapacity(capacity);
    }

    // not copyable for now
    StructureOfArraysBase(StructureOfArraysBase const& rhs) = delete;
    StructureOfArraysBase& operator=(StructureOfArraysBase const& rhs) = delete;

    // movability is trivial, so support it
    StructureOfArraysBase(StructureOfArraysBase&& rhs) noexcept {
        using std::swap;
        swap(mCapacity, rhs.mCapacity);
        swap(mSize, rhs.mSize);
        swap(mArrays, rhs.mArrays);
        swap(mAllocator, rhs.mAllocator);
    }

    StructureOfArraysBase& operator=(StructureOfArraysBase&& rhs) noexcept {
        if (this != &rhs) {
            using std::swap;
            swap(mCapacity, rhs.mCapacity);
            swap(mSize, rhs.mSize);
            swap(mArrays, rhs.mArrays);
            swap(mAllocator, rhs.mAllocator);
        }
        return *this;
    }

    ~StructureOfArraysBase() {
        destroy_each(0, mSize);
        mAllocator.free(std::get<0>(mArrays));
    }

    // --------------------------------------------------------------------------------------------

    // return the size the array
    size_t size() const noexcept {
        return mSize;
    }

    // return the capacity of the array
    size_t capacity() const noexcept {
        return mCapacity;
    }

    // set the capacity of the array. the capacity cannot be smaller than the current size,
    // the call is a no-op in that case.
    UTILS_NOINLINE
    void setCapacity(size_t capacity) {
        // allocate enough space for "capacity" elements of each array
        // capacity cannot change when optional storage is specified
        if (capacity >= mSize) {
            // TODO: not entirely sure if "max" of all alignments is always correct
            constexpr size_t align = std::max({ std::max(alignof(std::max_align_t), alignof(Elements))... });
            const size_t sizeNeeded = getNeededSize(capacity);
            void* buffer = mAllocator.alloc(sizeNeeded, align);
            auto const oldBuffer = std::get<0>(mArrays);

            // move all the items (one array at a time) from the old allocation to the new
            // this also update the array pointers
            move_each(buffer, capacity);

            // free the old buffer
            mAllocator.free(oldBuffer);

            // and make sure to update the capacity
            mCapacity = capacity;
        }
    }

    void ensureCapacity(size_t needed) {
        if (UTILS_UNLIKELY(needed > mCapacity)) {
            // not enough space, increase the capacity
            const size_t capacity = (needed * 3 + 1) / 2;
            setCapacity(capacity);
        }
    }

    // grow or shrink the array to the given size. When growing, new elements are constructed
    // with their default constructor. when shrinking, discarded elements are destroyed.
    // If the arrays don't have enough capacity, the capacity is increased accordingly
    // (the capacity is set to 3/2 of the asked size).
    UTILS_NOINLINE
    void resize(size_t needed) {
        ensureCapacity(needed);
        resizeNoCheck(needed);
        if (needed <= mCapacity) {
            // TODO: see if we should shrink the arrays
        }
    }

    void clear() noexcept {
        resizeNoCheck(0);
    }


    inline void swap(size_t i, size_t j) noexcept {
        forEach([i, j](auto p) {
            using std::swap;
            swap(p[i], p[j]);
        });
    }

    // remove and destroy the last element of each array
    inline void pop_back() noexcept {
        if (mSize) {
            destroy_each(mSize - 1, mSize);
            mSize--;
        }
    }

    // create an element at the end of each array
    StructureOfArraysBase& push_back() noexcept {
        resize(mSize + 1);
        return *this;
    }

    StructureOfArraysBase& push_back(Structure&& args) noexcept {
        ensureCapacity(mSize + 1);
        return push_back_unsafe(std::forward<Structure>(args));
    }

    StructureOfArraysBase& push_back(Elements const& ... args) noexcept {
        ensureCapacity(mSize + 1);
        return push_back_unsafe(args...);
    }

    StructureOfArraysBase& push_back(Elements&& ... args) noexcept {
        ensureCapacity(mSize + 1);
        return push_back_unsafe(std::forward<Elements>(args)...);
    }

    // in C++20 we could use a lambda with explicit template parameter instead
    struct PushBackUnsafeClosure {
        size_t last;
        std::tuple<Elements...> args;
        inline explicit PushBackUnsafeClosure(size_t last, Structure&& args)
                : last(last), args(std::forward<Structure>(args)) {}
        template<size_t I>
        inline void operator()(TypeAt<I>* p) {
            new(p + last) TypeAt<I>{ std::get<I>(args) };
        }
    };

    StructureOfArraysBase& push_back_unsafe(Structure&& args) noexcept {
        for_each_index(mArrays,
                PushBackUnsafeClosure{ mSize++, std::forward<Structure>(args) });
        return *this;
    }

    StructureOfArraysBase& push_back_unsafe(Elements const& ... args) noexcept {
        for_each_index(mArrays,
                PushBackUnsafeClosure{ mSize++, { args... } });
        return *this;
    }

    StructureOfArraysBase& push_back_unsafe(Elements&& ... args) noexcept {
        for_each_index(mArrays,
                PushBackUnsafeClosure{ mSize++, { std::forward<Elements>(args)... }});
        return *this;
    }

    template<typename F, typename ... ARGS>
    void forEach(F&& f, ARGS&& ... args) {
        for_each(mArrays, [&](size_t, auto* p) {
            f(p, std::forward<ARGS>(args)...);
        });
    }

    // return a pointer to the first element of the ElementIndex]th array
    template<size_t ElementIndex>
    TypeAt<ElementIndex>* data() noexcept {
        return std::get<ElementIndex>(mArrays);
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex> const* data() const noexcept {
        return std::get<ElementIndex>(mArrays);
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex>* begin() noexcept {
        return std::get<ElementIndex>(mArrays);
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex> const* begin() const noexcept {
        return std::get<ElementIndex>(mArrays);
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex>* end() noexcept {
        return std::get<ElementIndex>(mArrays) + size();
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex> const* end() const noexcept {
        return std::get<ElementIndex>(mArrays) + size();
    }

    template<size_t ElementIndex>
    Slice<TypeAt<ElementIndex>> slice() noexcept {
        return { begin<ElementIndex>(), end<ElementIndex>() };
    }

    template<size_t ElementIndex>
    Slice<const TypeAt<ElementIndex>> slice() const noexcept {
        return { begin<ElementIndex>(), end<ElementIndex>() };
    }

    // return a reference to the index'th element of the ElementIndex'th array
    template<size_t ElementIndex>
    TypeAt<ElementIndex>& elementAt(size_t index) noexcept {
        return data<ElementIndex>()[index];
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex> const& elementAt(size_t index) const noexcept {
        return data<ElementIndex>()[index];
    }

    // return a reference to the last element of the ElementIndex'th array
    template<size_t ElementIndex>
    TypeAt<ElementIndex>& back() noexcept {
        return data<ElementIndex>()[size() - 1];
    }

    template<size_t ElementIndex>
    TypeAt<ElementIndex> const& back() const noexcept {
        return data<ElementIndex>()[size() - 1];
    }

    template <size_t E, typename IndexType = uint32_t>
    struct Field {
        SoA& soa;
        IndexType i;
        using Type = typename SoA::template TypeAt<E>;

        UTILS_ALWAYS_INLINE Field& operator = (Field&& rhs) noexcept {
            soa.elementAt<E>(i) = soa.elementAt<E>(rhs.i);
            return *this;
        }

        // auto-conversion to the field's type
        UTILS_ALWAYS_INLINE operator Type&() noexcept {
            return soa.elementAt<E>(i);
        }
        UTILS_ALWAYS_INLINE operator Type const&() const noexcept {
            return soa.elementAt<E>(i);
        }
        // dereferencing the selected field
        UTILS_ALWAYS_INLINE Type& operator ->() noexcept {
            return soa.elementAt<E>(i);
        }
        UTILS_ALWAYS_INLINE Type const& operator ->() const noexcept {
            return soa.elementAt<E>(i);
        }
        // address-of the selected field
        UTILS_ALWAYS_INLINE Type* operator &() noexcept {
            return &soa.elementAt<E>(i);
        }
        UTILS_ALWAYS_INLINE Type const* operator &() const noexcept {
            return &soa.elementAt<E>(i);
        }
        // assignment to the field
        UTILS_ALWAYS_INLINE Type const& operator = (Type const& other) noexcept {
            return (soa.elementAt<E>(i) = other);
        }
        UTILS_ALWAYS_INLINE Type const& operator = (Type&& other) noexcept {
            return (soa.elementAt<E>(i) = other);
        }
        // comparisons
        UTILS_ALWAYS_INLINE bool operator==(Type const& other) const {
            return (soa.elementAt<E>(i) == other);
        }
        UTILS_ALWAYS_INLINE bool operator!=(Type const& other) const {
            return (soa.elementAt<E>(i) != other);
        }
        // calling the field
        template <typename ... ARGS>
        UTILS_ALWAYS_INLINE decltype(auto) operator()(ARGS&& ... args) noexcept {
            return soa.elementAt<E>(i)(std::forward<ARGS>(args)...);
        }
        template <typename ... ARGS>
        UTILS_ALWAYS_INLINE decltype(auto) operator()(ARGS&& ... args) const noexcept {
            return soa.elementAt<E>(i)(std::forward<ARGS>(args)...);
        }
    };

private:
    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    for_each(std::tuple<Tp...>&, FuncT) {}

    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    for_each(std::tuple<Tp...>& t, FuncT f) {
        f(I, std::get<I>(t));
        for_each<I + 1, FuncT, Tp...>(t, f);
    }

    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    for_each_index(std::tuple<Tp...>&, FuncT) {}

    template<std::size_t I = 0, typename FuncT, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    for_each_index(std::tuple<Tp...>& t, FuncT f) {
        f.template operator()<I>(std::get<I>(t));
        for_each_index<I + 1, FuncT, Tp...>(t, f);
    }

    inline void resizeNoCheck(size_t needed) noexcept {
        assert(mCapacity >= needed);
        if (needed < mSize) {
            // we shrink the arrays
            destroy_each(needed, mSize);
        } else if (needed > mSize) {
            // we grow the arrays
            construct_each(mSize, needed);
        }
        // record the new size of the arrays
        mSize = needed;
    }

    // this calculates the offset adjusted for all data alignment of a given array
    static inline size_t getOffset(size_t index, size_t capacity) noexcept {
        auto offsets = getOffsets(capacity);
        return offsets[index];
    }

    static inline std::array<size_t, kArrayCount> getOffsets(size_t capacity) noexcept {
        // compute the required size of each array
        const size_t sizes[] = { (sizeof(Elements) * capacity)... };

        // we align each array to at least the same alignment guaranteed by malloc
        constexpr size_t const alignments[] = { std::max(alignof(std::max_align_t), alignof(Elements))... };

        // hopefully most of this gets unrolled and inlined
        std::array<size_t, kArrayCount> offsets;
        offsets[0] = 0;
        UTILS_UNROLL
        for (size_t i = 1; i < kArrayCount; i++) {
            size_t unalignment = (offsets[i - 1] + sizes[i - 1]) % alignments[i];
            size_t alignment = unalignment ? (alignments[i] - unalignment) : 0;
            offsets[i] = offsets[i - 1] + (sizes[i - 1] + alignment);
            assert_invariant(offsets[i] % alignments[i] == 0);
        }
        return offsets;
    }

    void construct_each(size_t from, size_t to) noexcept {
        forEach([from, to](auto p) {
            using T = typename std::decay<decltype(*p)>::type;
            // note: scalar types like int/float get initialized to zero
            if constexpr (!std::is_trivially_default_constructible_v<T>) {
                for (size_t i = from; i < to; i++) {
                    new(p + i) T();
                }
            }
        });
    }

    void destroy_each(size_t from, size_t to) noexcept {
        forEach([from, to](auto p) {
            using T = typename std::decay<decltype(*p)>::type;
            if constexpr (!std::is_trivially_destructible_v<T>) {
                for (size_t i = from; i < to; i++) {
                    p[i].~T();
                }
            }
        });
    }

    void move_each(void* buffer, size_t capacity) noexcept {
        auto offsets = getOffsets(capacity);
        size_t index = 0;
        if (mSize) {
            auto size = mSize; // placate a compiler warning
            forEach([buffer, &index, &offsets, size](auto p) {
                using T = typename std::decay<decltype(*p)>::type;
                T* UTILS_RESTRICT b = static_cast<T*>(buffer);

                // go through each element and move them from the old array to the new
                // then destroy them from the old array
                T* UTILS_RESTRICT const arrayPointer =
                        reinterpret_cast<T*>(uintptr_t(b) + offsets[index]);

                // for trivial cases, just call memcpy()
                if constexpr (std::is_trivially_copyable_v<T> &&
                              std::is_trivially_destructible_v<T>) {
                    memcpy(arrayPointer, p, size * sizeof(T));
                } else {
                    for (size_t i = 0; i < size; i++) {
                        // we move an element by using the in-place move-constructor
                        new(arrayPointer + i) T(std::move(p[i]));
                        if constexpr (!std::is_trivially_destructible_v<T>) {
                            // and delete them by calling the destructor directly
                            p[i].~T();
                        }
                    }
                }
                index++;
            });
        }

        // update the pointers
        for_each(mArrays, [buffer, &offsets](size_t i, auto&& p) {
            using Type = std::remove_reference_t<decltype(p)>;
            p = Type((char*)buffer + offsets[i]);
        });
    }

    // capacity in array elements
    size_t mCapacity = 0;
    // size in array elements
    size_t mSize = 0;
    // N pointers to each arrays
    std::tuple<std::add_pointer_t<Elements>...> mArrays{};
    Allocator mAllocator;
};


template<typename Allocator, typename... Elements>
inline
typename StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef&
StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef::operator=(
        StructureOfArraysBase::IteratorValueRef const& rhs) {
    return operator=(IteratorValue(rhs));
}

template<typename Allocator, typename... Elements>
inline
typename StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef&
StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef::operator=(
        StructureOfArraysBase::IteratorValueRef&& rhs) noexcept {
    return operator=(IteratorValue(rhs));
}

template<typename Allocator, typename... Elements>
template<size_t... Is>
inline
typename StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef&
StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef::assign(
        StructureOfArraysBase::IteratorValue const& rhs, std::index_sequence<Is...>) {
    // implements IteratorValueRef& IteratorValueRef::operator=(IteratorValue const& rhs)
    auto UTILS_UNUSED l = { (soa->elementAt<Is>(index) = std::get<Is>(rhs.elements), 0)... };
    return *this;
}

template<typename Allocator, typename... Elements>
template<size_t... Is>
inline
typename StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef&
StructureOfArraysBase<Allocator, Elements...>::IteratorValueRef::assign(
        StructureOfArraysBase::IteratorValue&& rhs, std::index_sequence<Is...>) noexcept {
    // implements IteratorValueRef& IteratorValueRef::operator=(IteratorValue&& rhs) noexcept
    auto UTILS_UNUSED l = {
            (soa->elementAt<Is>(index) = std::move(std::get<Is>(rhs.elements)), 0)... };
    return *this;
}

template <typename ... Elements>
using StructureOfArrays = StructureOfArraysBase<HeapArena<>, Elements ...>;

} // namespace utils

#endif // TNT_UTILS_STRUCTUREOFARRAYS_H

