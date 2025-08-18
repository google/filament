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

#include "src/tint/lang/hlsl/type/rasterizer_ordered_texture_2d.h"

#include <cstddef>
#include <string>

#include "src/tint/lang/core/type/clone_context.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/unique_node.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/rtti/castable.h"

TINT_INSTANTIATE_TYPEINFO(tint::hlsl::type::RasterizerOrderedTexture2D);

namespace tint::hlsl::type {

RasterizerOrderedTexture2D::RasterizerOrderedTexture2D(core::TexelFormat format,
                                                       const core::type::Type* subtype)
    : Base(static_cast<size_t>(Hash(tint::TypeCode::Of<RasterizerOrderedTexture2D>().bits)),
           core::type::TextureDimension::k2d),
      texel_format_(format),
      subtype_(subtype) {}

bool RasterizerOrderedTexture2D::Equals(const UniqueNode& other) const {
    if (auto* o = other.As<RasterizerOrderedTexture2D>()) {
        return o->TexelFormat() == TexelFormat();
    }
    return false;
}

std::string RasterizerOrderedTexture2D::FriendlyName() const {
    StringStream out;
    out << "hlsl.rasterizer_ordered_texture_2d<" << ToString(TexelFormat()) << ">";
    return out.str();
}

const core::type::Type* RasterizerOrderedTexture2D::SubtypeFor(core::TexelFormat format,
                                                               core::type::Manager& type_mgr) {
    switch (format) {
        case core::TexelFormat::kR32Uint:
            return type_mgr.u32();
        case core::TexelFormat::kR32Sint:
            return type_mgr.i32();
        case core::TexelFormat::kR32Float:
            return type_mgr.f32();
        default:
            break;
    }
    return nullptr;
}

RasterizerOrderedTexture2D* RasterizerOrderedTexture2D::Clone(core::type::CloneContext& ctx) const {
    auto* ty = subtype_->Clone(ctx);
    return ctx.dst.mgr->Get<RasterizerOrderedTexture2D>(texel_format_, ty);
}

}  // namespace tint::hlsl::type
