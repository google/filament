// Copyright 2020 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_NATIVE_ENUMCLASSBITMASKS_H_
#define SRC_DAWN_NATIVE_ENUMCLASSBITMASKS_H_

#include <bit>

#include "webgpu/webgpu_enum_class_bitmasks.h"

namespace dawn::native {

// The operators of webgpu_enum_class_bitmasks.h are in the wgpu:: namespace,
// and need to be imported into this namespace for Argument Dependent Lookup.
WGPU_IMPORT_BITMASK_OPERATORS

// Specify this for usage with EnumMaskIterator
template <typename T>
struct EnumBitmaskSize {
    static constexpr unsigned value = 0;
};

template <typename T>
constexpr bool HasOneBit(T value) {
    using Integral = typename std::underlying_type<T>::type;
    return std::has_single_bit(static_cast<Integral>(value));
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ENUMCLASSBITMASKS_H_
