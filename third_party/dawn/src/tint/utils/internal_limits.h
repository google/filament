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

#ifndef SRC_TINT_UTILS_INTERNAL_LIMITS_H_
#define SRC_TINT_UTILS_INTERNAL_LIMITS_H_

#include <cstddef>
#include <cstdint>

// Contains constants for internal limits used within Tint. These limits are not specified in the
// WGSL spec, but are practical heuristics to limit certain operations that are known to cause
// memory or runtime issues, i.e. creation of arrays with 100k elements that cause IR binary
// decoding to take pathologically long times to run.

namespace tint::internal_limits {

// Limits the number of elements appearing in the constructor for an array
constexpr size_t kMaxArrayConstructorElements = 32767;

// Limits the number of elements in an array type
constexpr int64_t kMaxArrayElementCount = 65536;

// The max subgroup size supported. Used in validation.
constexpr int64_t kMaxSubgroupSize = 128;

// A quad (fragment) is composed of four invocations.
constexpr int64_t kQuadSize = 4;

}  // namespace tint::internal_limits

#endif  // SRC_TINT_UTILS_INTERNAL_LIMITS_H_
