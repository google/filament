// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/common/option_helpers.h"

#include <gtest/gtest.h>

#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/var.h"
#include "src/tint/lang/core/type/storage_texture.h"

namespace tint::hlsl::writer {
namespace {

using HlslWriterOptionHelpersTest = core::ir::IRTestHelper;

TEST_F(HlslWriterOptionHelpersTest, Empty) {
    Options options;
    auto res = ValidateBindingOptions(mod, options);
    EXPECT_EQ(res, Success);
}

TEST_F(HlslWriterOptionHelpersTest, SameMappedMultipleTimesToSameTarget) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{0, 1});
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{0, 1});
    options.bindings.storage.emplace(BindingPoint{0, 2}, BindingPoint{0, 3});
    auto res = ValidateBindingOptions(mod, options);
    EXPECT_EQ(res, Success);
}

TEST_F(HlslWriterOptionHelpersTest, ValidDisjointMappings) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{0, 1});
    options.bindings.storage.emplace(BindingPoint{0, 2}, BindingPoint{0, 3});
    options.bindings.texture.emplace(BindingPoint{0, 4}, BindingPoint{0, 5});
    options.bindings.sampler.emplace(BindingPoint{0, 6}, BindingPoint{0, 7});

    mod.root_block = b.Block();

    auto* ro_storage =
        b.Var("ro_storage", core::AddressSpace::kStorage, ty.i32(), core::Access::kRead);
    ro_storage->SetBindingPoint(0, 2);
    mod.root_block->Append(ro_storage);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_EQ(res, Success);
}

TEST_F(HlslWriterOptionHelpersTest, SameSourceDifferentTarget) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{0, 1});
    options.bindings.storage.emplace(BindingPoint{0, 0}, BindingPoint{0, 2});

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason,
              R"(error: found duplicate WGSL binding point: [group: 0, binding: 0]
note: when processing storage)");
}

TEST_F(HlslWriterOptionHelpersTest, UniformOverlaps) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.uniform.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing uniform)");
}

TEST_F(HlslWriterOptionHelpersTest, ExternalTextureMetadataOverlapsUniform) {
    Options options;
    options.bindings.uniform.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.external_texture.emplace(BindingPoint{0, 1}, ExternalMultiplanarTexture{
                                                                      BindingPoint{1, 1},
                                                                      BindingPoint{2, 2},
                                                                      BindingPoint{3, 3},
                                                                  });

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing external_texture)");
}

TEST_F(HlslWriterOptionHelpersTest, ReadOnlyStorageOverlapsTexture) {
    Options options;
    options.bindings.storage.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.texture.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    mod.root_block = b.Block();
    auto* ro_storage =
        b.Var("ro_storage", ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kRead));
    ro_storage->SetBindingPoint(0, 0);
    mod.root_block->Append(ro_storage);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing texture)");
}

TEST_F(HlslWriterOptionHelpersTest, ReadOnlyStorageTextureOverlapsTexture) {
    Options options;
    options.bindings.storage_texture.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.texture.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    mod.root_block = b.Block();
    auto* ro_storage_tex =
        b.Var("ro_st", ty.ptr(core::AddressSpace::kStorage,
                              ty.storage_texture(core::type::TextureDimension::k2d,
                                                 core::TexelFormat::kR32Float, core::Access::kRead),
                              core::Access::kRead));
    ro_storage_tex->SetBindingPoint(0, 0);
    mod.root_block->Append(ro_storage_tex);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing storage_texture)");
}

TEST_F(HlslWriterOptionHelpersTest, ExternalTexturePlane0OverlapsTexture) {
    Options options;
    options.bindings.texture.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.external_texture.emplace(BindingPoint{0, 1}, ExternalMultiplanarTexture{
                                                                      BindingPoint{4, 4},
                                                                      BindingPoint{1, 1},
                                                                      BindingPoint{3, 3},
                                                                  });

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing external_texture)");
}

TEST_F(HlslWriterOptionHelpersTest, ExternalTexturePlane1OverlapsTexture) {
    Options options;
    options.bindings.texture.emplace(BindingPoint{0, 0}, BindingPoint{2, 2});
    options.bindings.external_texture.emplace(BindingPoint{0, 1}, ExternalMultiplanarTexture{
                                                                      BindingPoint{4, 4},
                                                                      BindingPoint{1, 1},
                                                                      BindingPoint{2, 2},
                                                                  });

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 2]
note: when processing external_texture)");
}

TEST_F(HlslWriterOptionHelpersTest, ExternalTextureYCBCRTextureOverlapsTexture) {
    Options options;
    options.bindings.texture.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.external_texture.emplace(BindingPoint{0, 1}, ExternalYCBCRTexture{
                                                                      BindingPoint{4, 4},
                                                                      BindingPoint{1, 1},
                                                                      BindingPoint{3, 3},
                                                                  });

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing external_texture)");
}

TEST_F(HlslWriterOptionHelpersTest, TexelBufferOverlapsTexture) {
    Options options;
    options.bindings.texel_buffer.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.texture.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    mod.root_block = b.Block();
    auto* tb = b.Var("tb", ty.ptr(core::AddressSpace::kHandle,
                                  ty.Get<core::type::TexelBuffer>(core::TexelFormat::kR32Uint,
                                                                  core::Access::kRead, ty.u32())));
    tb->SetBindingPoint(0, 0);
    mod.root_block->Append(tb);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing texel_buffer)");
}

TEST_F(HlslWriterOptionHelpersTest, ReadWriteTexelBufferOverlaps) {
    Options options;
    options.bindings.texel_buffer.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.storage.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    mod.root_block = b.Block();
    auto* tb =
        b.Var("tb", ty.ptr(core::AddressSpace::kHandle,
                           ty.Get<core::type::TexelBuffer>(core::TexelFormat::kR32Uint,
                                                           core::Access::kReadWrite, ty.u32())));
    tb->SetBindingPoint(0, 0);
    mod.root_block->Append(tb);

    auto* rw_storage =
        b.Var("rw", ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite));
    rw_storage->SetBindingPoint(0, 1);
    mod.root_block->Append(rw_storage);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing texel_buffer)");
}

TEST_F(HlslWriterOptionHelpersTest, ReadWriteStorageOverlaps) {
    Options options;
    options.bindings.storage.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.storage.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    mod.root_block = b.Block();
    auto* rw_storage0 =
        b.Var("rw0", ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite));
    rw_storage0->SetBindingPoint(0, 0);
    mod.root_block->Append(rw_storage0);

    auto* rw_storage1 =
        b.Var("rw1", ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite));
    rw_storage1->SetBindingPoint(0, 1);
    mod.root_block->Append(rw_storage1);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing storage)");
}

TEST_F(HlslWriterOptionHelpersTest, WriteStorageTextureOverlaps) {
    Options options;
    options.bindings.storage.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.storage_texture.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    mod.root_block = b.Block();
    auto* rw_storage =
        b.Var("rw", ty.ptr(core::AddressSpace::kStorage, ty.i32(), core::Access::kReadWrite));
    rw_storage->SetBindingPoint(0, 0);
    mod.root_block->Append(rw_storage);

    auto* w_storage_tex =
        b.Var("w_st", ty.ptr(core::AddressSpace::kStorage,
                             ty.storage_texture(core::type::TextureDimension::k2d,
                                                core::TexelFormat::kR32Float, core::Access::kWrite),
                             core::Access::kRead));
    w_storage_tex->SetBindingPoint(0, 1);
    mod.root_block->Append(w_storage_tex);

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing storage_texture)");
}

TEST_F(HlslWriterOptionHelpersTest, SamplerOverlaps) {
    Options options;
    options.bindings.sampler.emplace(BindingPoint{0, 0}, BindingPoint{1, 1});
    options.bindings.sampler.emplace(BindingPoint{0, 1}, BindingPoint{1, 1});

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 1]
note: when processing sampler)");
}

TEST_F(HlslWriterOptionHelpersTest, ExternalTextureSamplerOverlaps) {
    Options options;
    options.bindings.sampler.emplace(BindingPoint{0, 0}, BindingPoint{2, 2});
    options.bindings.external_texture.emplace(
        BindingPoint{0, 1},
        ExternalYCBCRTexture{BindingPoint{4, 4}, BindingPoint{1, 1}, BindingPoint{2, 2}});

    auto res = ValidateBindingOptions(mod, options);
    EXPECT_NE(res, Success);
    EXPECT_EQ(res.Failure().reason, R"(error: found duplicate HLSL binding point: [binding: 2]
note: when processing external_texture)");
}

}  // namespace
}  // namespace tint::hlsl::writer
