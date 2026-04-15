// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_TINT_API_COMMON_BINDINGS_H_
#define SRC_TINT_API_COMMON_BINDINGS_H_

#include <unordered_map>

#include "src/tint/api/common/binding_point.h"
#include "src/tint/utils/reflection.h"

namespace tint {

/// A multiplanar external texture
struct ExternalMultiplanarTexture {
    /// Metadata
    BindingPoint metadata{};
    /// Plane0 binding data
    BindingPoint plane0{};
    /// Plane1 binding data
    BindingPoint plane1{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ExternalMultiplanarTexture, metadata, plane0, plane1);
    TINT_REFLECT_HASH_CODE(ExternalMultiplanarTexture);
    bool operator==(const ExternalMultiplanarTexture&) const = default;
};

/// A YCBcr external texture
struct ExternalYCBCRTexture {
    /// Metadata
    BindingPoint metadata{};
    /// texture binding data
    BindingPoint texture{};
    /// sampler binding data
    BindingPoint sampler{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(ExternalYCBCRTexture, metadata, texture, sampler);
    TINT_REFLECT_HASH_CODE(ExternalYCBCRTexture);
    bool operator==(const ExternalYCBCRTexture&) const = default;
};

using BindingMap = std::unordered_map<BindingPoint, BindingPoint>;
using ExternalTextureBindings =
    std::unordered_map<BindingPoint,
                       std::variant<ExternalMultiplanarTexture, ExternalYCBCRTexture>>;

/// Binding information
struct Bindings {
    /// Uniform bindings
    BindingMap uniform{};
    /// Storage bindings
    BindingMap storage{};
    /// Texture bindings
    BindingMap texture{};
    /// Storage texture bindings
    BindingMap storage_texture{};
    /// Texel buffer bindings
    BindingMap texel_buffer{};
    /// Sampler bindings
    BindingMap sampler{};
    /// External bindings
    ExternalTextureBindings external_texture{};
    /// Input attachment bindings
    BindingMap input_attachment{};

    /// Reflect the fields of this class so that it can be used by tint::ForeachField()
    TINT_REFLECT(Bindings,
                 uniform,
                 storage,
                 texture,
                 storage_texture,
                 texel_buffer,
                 sampler,
                 external_texture,
                 input_attachment);

    TINT_REFLECT_HASH_CODE(Bindings);

    bool operator==(const Bindings&) const = default;
};

}  // namespace tint

#endif  // SRC_TINT_API_COMMON_BINDINGS_H_
