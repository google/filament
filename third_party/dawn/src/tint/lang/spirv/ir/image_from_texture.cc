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

#include "src/tint/lang/spirv/ir/image_from_texture.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/input_attachment.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::spirv::ir {

const spirv::type::Image* ImageFromTexture(core::type::Manager& ty,
                                           const core::type::Texture* tex_ty) {
    auto dim = type::Dim::kD1;
    auto depth = type::Depth::kNotDepth;
    auto arrayed = type::Arrayed::kNonArrayed;
    auto ms = type::Multisampled::kSingleSampled;
    auto sampled = type::Sampled::kSamplingCompatible;
    auto fmt = core::TexelFormat::kUndefined;
    auto access = core::Access::kReadWrite;
    const core::type::Type* sample_ty = ty.f32();

    switch (tex_ty->Dim()) {
        case core::type::TextureDimension::k1d:
            dim = type::Dim::kD1;
            break;
        case core::type::TextureDimension::k2d:
            dim = type::Dim::kD2;
            break;
        case core::type::TextureDimension::k2dArray:
            dim = type::Dim::kD2;
            arrayed = type::Arrayed::kArrayed;
            break;
        case core::type::TextureDimension::k3d:
            dim = type::Dim::kD3;
            break;
        case core::type::TextureDimension::kCube:
            dim = type::Dim::kCube;
            break;
        case core::type::TextureDimension::kCubeArray:
            dim = type::Dim::kCube;
            arrayed = type::Arrayed::kArrayed;
            break;
        default:
            TINT_ICE() << "Invalid texture dimension: " << tex_ty->Dim();
    }

    tint::Switch(
        tex_ty,                                 //
        [&](const core::type::DepthTexture*) {  //
            depth = type::Depth::kDepth;
        },
        [&](const core::type::DepthMultisampledTexture*) {
            depth = type::Depth::kDepth;
            ms = type::Multisampled::kMultisampled;
        },
        [&](const core::type::MultisampledTexture* mt) {
            ms = type::Multisampled::kMultisampled;
            sample_ty = mt->Type();
        },
        [&](const core::type::SampledTexture* st) {
            sampled = type::Sampled::kSamplingCompatible;
            sample_ty = st->Type();
        },
        [&](const core::type::StorageTexture* st) {
            sampled = type::Sampled::kReadWriteOpCompatible;
            fmt = st->TexelFormat();
            sample_ty = st->Type();
            access = st->Access();
        },
        [&](const core::type::InputAttachment* ia) {
            dim = type::Dim::kSubpassData;
            sampled = type::Sampled::kReadWriteOpCompatible;
            sample_ty = ia->Type();
        },
        TINT_ICE_ON_NO_MATCH);

    return ty.Get<type::Image>(sample_ty, dim, depth, arrayed, ms, sampled, fmt, access);
}

}  // namespace tint::spirv::ir
