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

#ifndef SRC_TINT_UTILS_MEMORY_ALIGNED_STORAGE_H_
#define SRC_TINT_UTILS_MEMORY_ALIGNED_STORAGE_H_

#include <cstddef>

#include "src/tint/utils/memory/bitcast.h"

namespace tint {

/// A structure that has the same size and alignment as Entry.
/// Replacement for std::aligned_storage as this is broken on earlier versions of MSVC.
template <typename T>
struct alignas(alignof(T)) AlignedStorage {
    /// Byte array of length sizeof(T)
    std::byte data[sizeof(T)];

    /// @returns a pointer to aligned storage, reinterpreted as T&
    T& Get() { return *Bitcast<T*>(&data[0]); }

    /// @returns a pointer to aligned storage, reinterpreted as T&
    const T& Get() const { return *Bitcast<const T*>(&data[0]); }
};

}  // namespace tint

#endif  // SRC_TINT_UTILS_MEMORY_ALIGNED_STORAGE_H_
