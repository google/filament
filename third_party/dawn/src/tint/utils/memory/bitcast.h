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

#ifdef UNSAFE_BUFFERS_BUILD
// TODO(crbug.com/439062058): Remove this and convert code to safer constructs.
#pragma allow_unsafe_buffers
#endif

#ifndef SRC_TINT_UTILS_MEMORY_BITCAST_H_
#define SRC_TINT_UTILS_MEMORY_BITCAST_H_

#include <cstddef>
#include <cstring>
#include <type_traits>

namespace tint {

/// Bitcast performs a cast of `from` to the `TO` type using a memcpy.
/// This unsafe cast avoids triggering Clang's Control Flow Integrity checks.
/// See: crbug.com/dawn/1406
/// See: https://clang.llvm.org/docs/ControlFlowIntegrity.html#bad-cast-checking
/// @param from the value to cast
/// @tparam TO the value to cast to
/// @returns the cast value
template <typename TO, typename FROM>
inline TO Bitcast(FROM&& from) {
    static_assert(sizeof(FROM) == sizeof(TO));
    // gcc warns in cases where either TO or FROM are classes, even if they are trivially
    // copyable, with for example:
    //
    // error: ‘void* memcpy(void*, const void*, size_t)’ copying an object of
    // non-trivial type ‘struct tint::Number<unsigned int>’ from an array of ‘float’
    // [-Werror=class-memaccess]
    //
    // We avoid this by asserting that both types are indeed trivially copyable, and casting both
    // args to std::byte*.
    static_assert(std::is_trivially_copyable_v<std::decay_t<FROM>>);
    static_assert(std::is_trivially_copyable_v<std::decay_t<TO>>);
    TO to;
    memcpy(reinterpret_cast<std::byte*>(&to), reinterpret_cast<const std::byte*>(&from),
           sizeof(TO));
    return to;
}

}  // namespace tint

#endif  // SRC_TINT_UTILS_MEMORY_BITCAST_H_
