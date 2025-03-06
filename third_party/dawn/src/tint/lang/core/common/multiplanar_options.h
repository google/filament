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

#ifndef SRC_TINT_LANG_CORE_COMMON_MULTIPLANAR_OPTIONS_H_
#define SRC_TINT_LANG_CORE_COMMON_MULTIPLANAR_OPTIONS_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"

namespace tint::transform::multiplanar {

/// This struct identifies the binding groups and locations for new bindings to
/// use when transforming a texture_external instance.
struct BindingPoints {
    /// The desired binding location of the texture_2d representing plane #1 when
    /// a texture_external binding is expanded.
    BindingPoint plane_1;
    /// The desired binding location of the ExternalTextureParams uniform when a
    /// texture_external binding is expanded.
    BindingPoint params;

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(BindingPoints, plane_1, params);
};

/// BindingsMap is a map where the key is the binding location of a
/// texture_external and the value is a struct containing the desired
/// locations for new bindings expanded from the texture_external instance.
using BindingsMap = std::unordered_map<BindingPoint, BindingPoints>;

}  // namespace tint::transform::multiplanar

#endif  // SRC_TINT_LANG_CORE_COMMON_MULTIPLANAR_OPTIONS_H_
