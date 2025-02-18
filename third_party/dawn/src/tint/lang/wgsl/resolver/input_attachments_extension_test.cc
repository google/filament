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

#include "gmock/gmock.h"
#include "src/tint/lang/core/number.h"
#include "src/tint/lang/wgsl/extension.h"
#include "src/tint/lang/wgsl/resolver/resolver.h"
#include "src/tint/lang/wgsl/resolver/resolver_helper_test.h"

namespace tint::resolver {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

using InputAttachmenExtensionTest = ResolverTest;

// Test that input_attachment cannot be used without extension.
TEST_F(InputAttachmenExtensionTest, InputAttachmentWithoutExtension) {
    // @group(0) @binding(0) @input_attachment_index(3)
    // var input_tex : input_attachment<f32>;

    GlobalVar("input_tex", ty.input_attachment(ty.Of<f32>()),
              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(3_u)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of 'input_attachment' requires enabling extension 'chromium_internal_input_attachments')");
}

// Test that input_attachment cannot be declared locally.
TEST_F(InputAttachmenExtensionTest, InputAttachmentLocalDecl) {
    // enable chromium_internal_input_attachments;
    // @fragment fn f() {
    //    var input_tex : input_attachment<f32>;
    // }

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    Func("f", Empty, ty.void_(),
         Vector{
             Decl(Var("input_tex", ty.input_attachment(ty.Of<f32>()))),
         },
         Vector{Stage(ast::PipelineStage::kFragment)});
    EXPECT_FALSE(r()->Resolve());
}

// Test that input_attachment cannot be declared without index.
TEST_F(InputAttachmenExtensionTest, InputAttachmentWithoutIndex) {
    // enable chromium_internal_input_attachments;
    // @group(0) @binding(0)
    // var input_tex : input_attachment<f32>;

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    GlobalVar("input_tex", ty.input_attachment(ty.Of<f32>()), Vector{Binding(0_u), Group(0_u)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(),
              R"(error: 'input_attachment' variables require '@input_attachment_index' attribute)");
}

// Test that Resolver can get input_attachment_index value.
TEST_F(InputAttachmenExtensionTest, InputAttachmentIndexValue) {
    // enable chromium_internal_input_attachments;
    // @group(0) @binding(0) @input_attachment_index(3)
    // var input_tex : input_attachment<f32>;

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    auto* ast_var = GlobalVar("input_tex", ty.input_attachment(ty.Of<f32>()),
                              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(3_u)});

    EXPECT_TRUE(r()->Resolve()) << r()->error();

    auto* sem_var = Sem().Get<sem::GlobalVariable>(ast_var);
    ASSERT_NE(sem_var, nullptr);
    EXPECT_EQ(sem_var->Attributes().input_attachment_index, 3u);
}

// Test that @input_attachment_index cannot be used without extension.
TEST_F(InputAttachmenExtensionTest, InputAttachmentIndexWithoutExtension) {
    // @group(0) @binding(0) @input_attachment_index(3)
    // var input_tex : texture_2d<f32>;

    GlobalVar("input_tex", ty.sampled_texture(core::type::TextureDimension::k2d, ty.Of<f32>()),
              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(3_u)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: use of '@input_attachment_index' requires enabling extension 'chromium_internal_input_attachments')");
}

// Test that input_attachment_index's value cannot be float.
TEST_F(InputAttachmenExtensionTest, InputAttachmentIndexInvalidValueType) {
    // enable chromium_internal_input_attachments;
    // @group(0) @binding(0) @input_attachment_index(3.0)
    // var input_tex : input_attachment<f32>;

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    GlobalVar("input_tex", ty.input_attachment(ty.Of<f32>()),
              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(3_f)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: '@input_attachment_index' must be an 'i32' or 'u32' value)");
}

// Test that input_attachment_index's value cannot be negative.
TEST_F(InputAttachmenExtensionTest, InputAttachmentIndexNegative) {
    // enable chromium_internal_input_attachments;
    // @group(0) @binding(0) @input_attachment_index(-2)
    // var input_tex : input_attachment<f32>;

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    GlobalVar("input_tex", ty.input_attachment(ty.Of<f32>()),
              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(core::i32(-2))});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(r()->error(), R"(error: '@input_attachment_index' value must be non-negative)");
}

// Test that input_attachment_index cannot be used on non input_attachment variable.
TEST_F(InputAttachmenExtensionTest, InputAttachmentIndexInvalidType) {
    // enable chromium_internal_input_attachments;
    // @group(0) @binding(0) @input_attachment_index(3)
    // var input_tex : texture_2d<f32>;

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    GlobalVar("input_tex", ty.sampled_texture(core::type::TextureDimension::k2d, ty.Of<f32>()),
              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(3_u)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_EQ(
        r()->error(),
        R"(error: cannot apply '@input_attachment_index' to declaration of type 'texture_2d<f32>'
note: '@input_attachment_index' must only be applied to declarations of 'input_attachment' type)");
}

// Test that inputAttachmentLoad cannot be used in vertex shader
TEST_F(InputAttachmenExtensionTest, InputAttachmentLoadInVertexStageError) {
    // enable chromium_internal_input_attachments;
    // @group(0) @binding(0) @input_attachment_index(3)
    // var input_tex : input_attachment<f32>;
    // @vertex fn f() -> @builtin(position) vec4f {
    //    return inputAttachmentLoad(input_tex);
    // }

    Enable(Source{{12, 34}}, wgsl::Extension::kChromiumInternalInputAttachments);

    GlobalVar("input_tex", ty.input_attachment(ty.Of<f32>()),
              Vector{Binding(0_u), Group(0_u), InputAttachmentIndex(3_u)});

    Func("f", Empty, ty.vec4<f32>(),
         Vector{
             Return(Call("inputAttachmentLoad", "input_tex")),
         },
         Vector{Stage(ast::PipelineStage::kVertex)},
         Vector{Builtin(core::BuiltinValue::kPosition)});

    EXPECT_FALSE(r()->Resolve());
    EXPECT_NE(r()->error().find(R"(cannot be used by vertex pipeline stage)"), std::string::npos);
}

}  // namespace
}  // namespace tint::resolver
