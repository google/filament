/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_UTILS_MEMALIGN_H
#define TNT_UTILS_MEMALIGN_H

#include <type_traits>

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#if defined(WIN32)
#include <malloc.h>
#endif

namespace utils {

inline void* aligned_alloc(size_t size, size_t align) noexcept {
    // 'align' must be a power of two and a multiple of sizeof(void*)
    align = (align < sizeof(void*)) ? sizeof(void*) : align;
    assert(align && !(align & align - 1));
    assert((align % sizeof(void*)) == 0);

    void* p = nullptr;

#if defined(WIN32)
    p = ::_aligned_malloc(size, align);
#else
    ::posix_memalign(&p, align, size);
#endif
    return p;
}

inline void aligned_free(void* p) noexcept {
#if defined(WIN32)
    ::_aligned_free(p);
#else
    ::free(p);
#endif
}

/*
 * This allocator can be used with std::vector for instance to ensure all items are aligned
 * to their alignof(). e.g.
 *
 *      template<typename T>
 *      using aligned_vector = std::vector<T, utils::STLAlignedAllocator<T>>;
 *
 *      aligned_vector<Foo> foos;
 *
 */
template<typename TYPE>
class STLAlignedAllocator {
    static_assert(!(alignof(TYPE) & (alignof(TYPE) - 1)), "alignof(T) must be a power of two");

public:
    using value_type = TYPE;
    using pointer = TYPE*;
    using const_pointer = const TYPE*;
    using reference = TYPE&;
    using const_reference = const TYPE&;
    using size_type = ::size_t;
    using difference_type = ::ptrdiff_t;
    using propagate_on_container_move_assignment = std::true_type;
    using is_always_equal = std::true_type;

    template<typename T>
    struct rebind { using other = STLAlignedAllocator<T>; };

    inline STLAlignedAllocator() noexcept = default;

    template<typename T>
    inline explicit STLAlignedAllocator(const STLAlignedAllocator<T>&) noexcept {}

    inline ~STLAlignedAllocator() noexcept = default;

    inline pointer allocate(size_type n) noexcept {
        return (pointer)aligned_alloc(n * sizeof(value_type), alignof(TYPE));
    }

    inline void deallocate(pointer p, size_type) {
        aligned_free(p);
    }

    // stateless allocators are always equal
    template<typename T>
    bool operator==(const STLAlignedAllocator<T>&) const noexcept {
        return true;
    }

    template<typename T>
    bool operator!=(const STLAlignedAllocator<T>&) const noexcept {
        return false;
    }
};

} // namespace utils

#endif // TNT_UTILS_MEMALIGN_H
