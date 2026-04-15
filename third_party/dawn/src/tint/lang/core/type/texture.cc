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

#include "src/tint/lang/core/type/texture.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::type::Texture);

namespace tint::core::type {

Texture::Texture(size_t hash, TextureDimension dim) : Base(hash, core::type::Flags{}), dim_(dim) {}

Texture::~Texture() = default;

bool IsTextureArray(core::type::TextureDimension dim) {
    switch (dim) {
        case core::type::TextureDimension::k2dArray:
        case core::type::TextureDimension::kCubeArray:
            return true;
        case core::type::TextureDimension::k2d:
        case core::type::TextureDimension::kNone:
        case core::type::TextureDimension::k1d:
        case core::type::TextureDimension::k3d:
        case core::type::TextureDimension::kCube:
            return false;
    }
    return false;
}

int NumCoordinateAxes(core::type::TextureDimension dim) {
    switch (dim) {
        case core::type::TextureDimension::kNone:
            return 0;
        case core::type::TextureDimension::k1d:
            return 1;
        case core::type::TextureDimension::k2d:
        case core::type::TextureDimension::k2dArray:
            return 2;
        case core::type::TextureDimension::k3d:
        case core::type::TextureDimension::kCube:
        case core::type::TextureDimension::kCubeArray:
            return 3;
    }
    return 0;
}

uint32_t Texture::Align() const {
    return 1;
}

}  // namespace tint::core::type
