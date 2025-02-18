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

#ifndef SRC_DAWN_NATIVE_OPENGL_BINDPOINT_H_
#define SRC_DAWN_NATIVE_OPENGL_BINDPOINT_H_

#include <utility>

#include "absl/container/flat_hash_map.h"
#include "src/tint/api/common/binding_point.h"

namespace dawn::native::opengl {

/// Indicate the type of field for each entry to push.
enum class BindPointFunction : uint8_t {
    /// The number of MIP levels of the bound texture view.
    kTextureNumLevels,
    /// The number of samples per texel of the bound multi-sampled texture.
    kTextureNumSamples,
};

/// Records the field and the byte offset of the data to push in the internal uniform buffer.
using FunctionAndOffset = std::pair<BindPointFunction, uint32_t>;
/// Maps from binding point to data entry with the information to populate the data.
using BindingPointToFunctionAndOffset = absl::flat_hash_map<tint::BindingPoint, FunctionAndOffset>;

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_BINDPOINT_H_
