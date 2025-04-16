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

#include "src/tint/lang/glsl/ir/combined_texture_sampler_var.h"

#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/sampled_texture.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::glsl::ir {
namespace {

using namespace tint::core::number_suffixes;  // NOLINT
                                              //
using IR_GlslCombinedTextureSamplerTest = core::ir::IRTestHelper;

TEST_F(IR_GlslCombinedTextureSamplerTest, Clone) {
    BindingPoint texture_bp{1, 2};
    BindingPoint sampler_bp{3, 4};
    auto* type = ty.ptr<handle>(ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32()));
    auto* var = mod.CreateInstruction<CombinedTextureSamplerVar>(b.InstructionResult(type),
                                                                 texture_bp, sampler_bp);

    auto* new_var = clone_ctx.Clone(var);

    EXPECT_NE(var, new_var);
    EXPECT_NE(var->Result(), new_var->Result());
    EXPECT_EQ(new_var->Result()->Type(), type);

    EXPECT_EQ(new_var->TextureBindingPoint(), texture_bp);
    EXPECT_EQ(new_var->SamplerBindingPoint(), sampler_bp);
}

}  // namespace
}  // namespace tint::glsl::ir
