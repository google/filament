// Copyright 2024 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_ALIGNED_ALLOC_H_
#define SRC_DAWN_COMMON_ALIGNED_ALLOC_H_

#include <cstdlib>

namespace dawn {

// The implementations of dawn::AlignedAlloc() and dawn::AlignedFree() are referenced from
// base::AlignedAlloc() and base::AlignedFree() in Chromium (in `base/memory/aligned_memory.h`).
//
// In Dawn, a runtime sized aligned allocation can be created:
//
//   float* my_array = static_cast<float*>(AlignedAlloc(size, alignment));
//   CHECK(reinterpret_cast<uintptr_t>(my_array) % alignment == 0);
//   memset(my_array, 0, size);  // fills entire object.
//
//   // ... later, to release the memory:
//   AlignedFree(my_array);
//
// Or using unique_ptr:
//
//   std::unique_ptr<float, AlignedFreeDeleter> my_array(
//       static_cast<float*>(AlignedAlloc(size, alignment)));

// Allocate memory of size `size` aligned to `alignment`.
// The `alignment` parameter must be an integer power of 2.
// The `size` parameter must be an integral multiple of `alignment`.
// Note that the memory allocated by `dawn::AlignedAlloc()` can only be deallocated by
// `dawn::AlignedFree()` otherwise on Windows the aligned memory may not be reclaimed correctly.
void* AlignedAlloc(size_t size, size_t alignment);

// Deallocate memory allocated by `AlignedAlloc`.
void AlignedFree(void* alignedPtr);

// Deleter for use with unique_ptr. E.g., use as
//   std::unique_ptr<Foo, dawn::AlignedFreeDeleter> foo;
struct AlignedFreeDeleter {
    inline void operator()(void* ptr) const { AlignedFree(ptr); }
};

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_ALIGNED_ALLOC_H_
