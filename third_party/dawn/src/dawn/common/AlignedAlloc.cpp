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

#include "dawn/common/AlignedAlloc.h"

#include <cstdlib>
#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"

namespace dawn {

void* AlignedAlloc(size_t size, size_t alignment) {
    DAWN_ASSERT(size > 0);
    DAWN_ASSERT(IsPowerOfTwo(alignment));
    DAWN_ASSERT(size % alignment == 0);

#if DAWN_PLATFORM_IS(WINDOWS)
    return _aligned_malloc(size, alignment);

#elif DAWN_PLATFORM_IS(ANDROID)
    // Currently std::aligned_alloc() is not supported on the Android build of Chromium. Luckily,
    // memalign() on Android returns pointers which can safely be used with free(), so we can use it
    // instead.
    return memalign(alignment, size);

#else
    return aligned_alloc(alignment, size);

#endif
}

void AlignedFree(void* alignedPtr) {
#if DAWN_PLATFORM_IS(WINDOWS)
    _aligned_free(alignedPtr);

#else
    free(alignedPtr);

#endif
}

}  // namespace dawn
