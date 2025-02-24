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

#include "gtest/gtest.h"

#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/spirv/reader/ast_parser/type.h"

namespace tint::spirv::reader::ast_parser {
namespace {

TEST(SpirvASTParserTypeTest, SameArgumentsGivesSamePointer) {
    Symbol sym(Symbol(1, {}, "1"));

    TypeManager ty;
    EXPECT_EQ(ty.Void(), ty.Void());
    EXPECT_EQ(ty.Bool(), ty.Bool());
    EXPECT_EQ(ty.U32(), ty.U32());
    EXPECT_EQ(ty.F32(), ty.F32());
    EXPECT_EQ(ty.F16(), ty.F16());
    EXPECT_EQ(ty.I32(), ty.I32());
    EXPECT_EQ(ty.Pointer(core::AddressSpace::kUndefined, ty.I32()),
              ty.Pointer(core::AddressSpace::kUndefined, ty.I32()));
    EXPECT_EQ(ty.Vector(ty.I32(), 3), ty.Vector(ty.I32(), 3));
    EXPECT_EQ(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 3, 2));
    EXPECT_EQ(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 3, 2));
    EXPECT_EQ(ty.Alias(sym, ty.I32()), ty.Alias(sym, ty.I32()));
    EXPECT_EQ(ty.Struct(sym, {ty.I32()}), ty.Struct(sym, {ty.I32()}));
    EXPECT_EQ(ty.Sampler(core::type::SamplerKind::kSampler),
              ty.Sampler(core::type::SamplerKind::kSampler));
    EXPECT_EQ(ty.DepthTexture(core::type::TextureDimension::k2d),
              ty.DepthTexture(core::type::TextureDimension::k2d));
    EXPECT_EQ(ty.MultisampledTexture(core::type::TextureDimension::k2d, ty.I32()),
              ty.MultisampledTexture(core::type::TextureDimension::k2d, ty.I32()));
    EXPECT_EQ(ty.SampledTexture(core::type::TextureDimension::k2d, ty.I32()),
              ty.SampledTexture(core::type::TextureDimension::k2d, ty.I32()));
    EXPECT_EQ(ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kRead),
              ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kRead));
}

TEST(SpirvASTParserTypeTest, DifferentArgumentsGivesDifferentPointer) {
    Symbol sym_a(Symbol(1, {}, "1"));
    Symbol sym_b(Symbol(2, {}, "2"));

    TypeManager ty;
    EXPECT_NE(ty.Pointer(core::AddressSpace::kUndefined, ty.I32()),
              ty.Pointer(core::AddressSpace::kUndefined, ty.U32()));
    EXPECT_NE(ty.Pointer(core::AddressSpace::kUndefined, ty.I32()),
              ty.Pointer(core::AddressSpace::kIn, ty.I32()));
    EXPECT_NE(ty.Vector(ty.I32(), 3), ty.Vector(ty.U32(), 3));
    EXPECT_NE(ty.Vector(ty.I32(), 3), ty.Vector(ty.I32(), 2));
    EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.U32(), 3, 2));
    EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 2, 2));
    EXPECT_NE(ty.Matrix(ty.I32(), 3, 2), ty.Matrix(ty.I32(), 3, 3));
    EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.U32(), 3, 2));
    EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 2, 2));
    EXPECT_NE(ty.Array(ty.I32(), 3, 2), ty.Array(ty.I32(), 3, 3));
    EXPECT_NE(ty.Alias(sym_a, ty.I32()), ty.Alias(sym_b, ty.I32()));
    EXPECT_NE(ty.Struct(sym_a, {ty.I32()}), ty.Struct(sym_b, {ty.I32()}));
    EXPECT_NE(ty.Sampler(core::type::SamplerKind::kSampler),
              ty.Sampler(core::type::SamplerKind::kComparisonSampler));
    EXPECT_NE(ty.DepthTexture(core::type::TextureDimension::k2d),
              ty.DepthTexture(core::type::TextureDimension::k1d));
    EXPECT_NE(ty.MultisampledTexture(core::type::TextureDimension::k2d, ty.I32()),
              ty.MultisampledTexture(core::type::TextureDimension::k3d, ty.I32()));
    EXPECT_NE(ty.MultisampledTexture(core::type::TextureDimension::k2d, ty.I32()),
              ty.MultisampledTexture(core::type::TextureDimension::k2d, ty.U32()));
    EXPECT_NE(ty.SampledTexture(core::type::TextureDimension::k2d, ty.I32()),
              ty.SampledTexture(core::type::TextureDimension::k3d, ty.I32()));
    EXPECT_NE(ty.SampledTexture(core::type::TextureDimension::k2d, ty.I32()),
              ty.SampledTexture(core::type::TextureDimension::k2d, ty.U32()));
    EXPECT_NE(ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kRead),
              ty.StorageTexture(core::type::TextureDimension::k3d, core::TexelFormat::kR32Uint,
                                core::Access::kRead));
    EXPECT_NE(ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kRead),
              ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Sint,
                                core::Access::kRead));
    EXPECT_NE(ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kRead),
              ty.StorageTexture(core::type::TextureDimension::k2d, core::TexelFormat::kR32Uint,
                                core::Access::kWrite));
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
