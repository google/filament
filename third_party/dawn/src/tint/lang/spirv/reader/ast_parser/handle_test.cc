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

#include <ostream>

#include "gmock/gmock.h"
#include "src/tint/lang/spirv/reader/ast_parser/function.h"
#include "src/tint/lang/spirv/reader/ast_parser/helper_test.h"
#include "src/tint/lang/spirv/reader/ast_parser/spirv_tools_helpers_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::spirv::reader::ast_parser {
namespace {

using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::StartsWith;

using SpvParserHandleTest = SpirvASTParserTest;

std::string Preamble() {
    return R"(
    OpCapability Shader
    OpCapability Sampled1D
    OpCapability Image1D
    OpCapability StorageImageExtendedFormats
    OpCapability ImageQuery
    OpMemoryModel Logical Simple
  )";
}

std::string FragMain() {
    return R"(
    OpEntryPoint Fragment %main "main" ; assume no IO
    OpExecutionMode %main OriginUpperLeft
  )";
}

std::string MainBody() {
    return R"(
    %main = OpFunction %void None %voidfn
    %main_entry = OpLabel
    OpReturn
    OpFunctionEnd
  )";
}

std::string CommonBasicTypes() {
    return R"(
    %void = OpTypeVoid
    %voidfn = OpTypeFunction %void

    %float = OpTypeFloat 32
    %uint = OpTypeInt 32 0
    %int = OpTypeInt 32 1

    %int_0 = OpConstant %int 0
    %int_1 = OpConstant %int 1
    %int_2 = OpConstant %int 2
    %int_3 = OpConstant %int 3
    %int_4 = OpConstant %int 4
    %uint_0 = OpConstant %uint 0
    %uint_1 = OpConstant %uint 1
    %uint_2 = OpConstant %uint 2
    %uint_3 = OpConstant %uint 3
    %uint_4 = OpConstant %uint 4
    %uint_100 = OpConstant %uint 100

    %v2int = OpTypeVector %int 2
    %v3int = OpTypeVector %int 3
    %v4int = OpTypeVector %int 4
    %v2uint = OpTypeVector %uint 2
    %v3uint = OpTypeVector %uint 3
    %v4uint = OpTypeVector %uint 4
    %v2float = OpTypeVector %float 2
    %v3float = OpTypeVector %float 3
    %v4float = OpTypeVector %float 4

    %float_null = OpConstantNull %float
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
    %float_7 = OpConstant %float 7
    %v2float_null = OpConstantNull %v2float
    %v3float_null = OpConstantNull %v3float
    %v4float_null = OpConstantNull %v4float

    %the_vi12 = OpConstantComposite %v2int %int_1 %int_2
    %the_vi123 = OpConstantComposite %v3int %int_1 %int_2 %int_3
    %the_vi1234 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4

    %the_vu12 = OpConstantComposite %v2uint %uint_1 %uint_2
    %the_vu123 = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3
    %the_vu1234 = OpConstantComposite %v4uint %uint_1 %uint_2 %uint_3 %uint_4

    %the_vf12 = OpConstantComposite %v2float %float_1 %float_2
    %the_vf21 = OpConstantComposite %v2float %float_2 %float_1
    %the_vf123 = OpConstantComposite %v3float %float_1 %float_2 %float_3
    %the_vf1234 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4


    %depth = OpConstant %float 0.2
  )";
}

std::string CommonImageTypes() {
    return R"(

; Define types for all sampler and texture types that can map to WGSL,
; modulo texel formats for storage textures. For now, we limit
; ourselves to 2-channel 32-bit texel formats.

; Because the SPIR-V reader also already generalizes so it can work with
; combined image-samplers, we also test that too.

    %sampler = OpTypeSampler

    ; sampled images
    %f_texture_1d          = OpTypeImage %float 1D   0 0 0 1 Unknown
    %f_texture_2d          = OpTypeImage %float 2D   0 0 0 1 Unknown
    %f_texture_2d_ms       = OpTypeImage %float 2D   0 0 1 1 Unknown
    %f_texture_2d_array    = OpTypeImage %float 2D   0 1 0 1 Unknown
    %f_texture_2d_ms_array = OpTypeImage %float 2D   0 1 1 1 Unknown ; not in WebGPU
    %f_texture_3d          = OpTypeImage %float 3D   0 0 0 1 Unknown
    %f_texture_cube        = OpTypeImage %float Cube 0 0 0 1 Unknown
    %f_texture_cube_array  = OpTypeImage %float Cube 0 1 0 1 Unknown

    ; storage images
    %f_storage_1d         = OpTypeImage %float 1D   0 0 0 2 Rg32f
    %f_storage_2d         = OpTypeImage %float 2D   0 0 0 2 Rg32f
    %f_storage_2d_array   = OpTypeImage %float 2D   0 1 0 2 Rg32f
    %f_storage_3d         = OpTypeImage %float 3D   0 0 0 2 Rg32f

    ; Now all the same, but for unsigned integer sampled type.

    %u_texture_1d          = OpTypeImage %uint  1D   0 0 0 1 Unknown
    %u_texture_2d          = OpTypeImage %uint  2D   0 0 0 1 Unknown
    %u_texture_2d_ms       = OpTypeImage %uint  2D   0 0 1 1 Unknown
    %u_texture_2d_array    = OpTypeImage %uint  2D   0 1 0 1 Unknown
    %u_texture_2d_ms_array = OpTypeImage %uint  2D   0 1 1 1 Unknown ; not in WebGPU
    %u_texture_3d          = OpTypeImage %uint  3D   0 0 0 1 Unknown
    %u_texture_cube        = OpTypeImage %uint  Cube 0 0 0 1 Unknown
    %u_texture_cube_array  = OpTypeImage %uint  Cube 0 1 0 1 Unknown

    %u_storage_1d         = OpTypeImage %uint  1D   0 0 0 2 Rg32ui
    %u_storage_2d         = OpTypeImage %uint  2D   0 0 0 2 Rg32ui
    %u_storage_2d_array   = OpTypeImage %uint  2D   0 1 0 2 Rg32ui
    %u_storage_3d         = OpTypeImage %uint  3D   0 0 0 2 Rg32ui

    ; Now all the same, but for signed integer sampled type.

    %i_texture_1d          = OpTypeImage %int  1D   0 0 0 1 Unknown
    %i_texture_2d          = OpTypeImage %int  2D   0 0 0 1 Unknown
    %i_texture_2d_ms       = OpTypeImage %int  2D   0 0 1 1 Unknown
    %i_texture_2d_array    = OpTypeImage %int  2D   0 1 0 1 Unknown
    %i_texture_2d_ms_array = OpTypeImage %int  2D   0 1 1 1 Unknown ; not in WebGPU
    %i_texture_3d          = OpTypeImage %int  3D   0 0 0 1 Unknown
    %i_texture_cube        = OpTypeImage %int  Cube 0 0 0 1 Unknown
    %i_texture_cube_array  = OpTypeImage %int  Cube 0 1 0 1 Unknown

    %i_storage_1d         = OpTypeImage %int  1D   0 0 0 2 Rg32i
    %i_storage_2d         = OpTypeImage %int  2D   0 0 0 2 Rg32i
    %i_storage_2d_array   = OpTypeImage %int  2D   0 1 0 2 Rg32i
    %i_storage_3d         = OpTypeImage %int  3D   0 0 0 2 Rg32i

    ;; Now pointers to each of the above, so we can declare variables for them.

    %ptr_sampler = OpTypePointer UniformConstant %sampler

    %ptr_f_texture_1d          = OpTypePointer UniformConstant %f_texture_1d
    %ptr_f_texture_2d          = OpTypePointer UniformConstant %f_texture_2d
    %ptr_f_texture_2d_ms       = OpTypePointer UniformConstant %f_texture_2d_ms
    %ptr_f_texture_2d_array    = OpTypePointer UniformConstant %f_texture_2d_array
    %ptr_f_texture_2d_ms_array = OpTypePointer UniformConstant %f_texture_2d_ms_array
    %ptr_f_texture_3d          = OpTypePointer UniformConstant %f_texture_3d
    %ptr_f_texture_cube        = OpTypePointer UniformConstant %f_texture_cube
    %ptr_f_texture_cube_array  = OpTypePointer UniformConstant %f_texture_cube_array

    ; storage images
    %ptr_f_storage_1d         = OpTypePointer UniformConstant %f_storage_1d
    %ptr_f_storage_2d         = OpTypePointer UniformConstant %f_storage_2d
    %ptr_f_storage_2d_array   = OpTypePointer UniformConstant %f_storage_2d_array
    %ptr_f_storage_3d         = OpTypePointer UniformConstant %f_storage_3d

    ; Now all the same, but for unsigned integer sampled type.

    %ptr_u_texture_1d          = OpTypePointer UniformConstant %u_texture_1d
    %ptr_u_texture_2d          = OpTypePointer UniformConstant %u_texture_2d
    %ptr_u_texture_2d_ms       = OpTypePointer UniformConstant %u_texture_2d_ms
    %ptr_u_texture_2d_array    = OpTypePointer UniformConstant %u_texture_2d_array
    %ptr_u_texture_2d_ms_array = OpTypePointer UniformConstant %u_texture_2d_ms_array
    %ptr_u_texture_3d          = OpTypePointer UniformConstant %u_texture_3d
    %ptr_u_texture_cube        = OpTypePointer UniformConstant %u_texture_cube
    %ptr_u_texture_cube_array  = OpTypePointer UniformConstant %u_texture_cube_array

    %ptr_u_storage_1d         = OpTypePointer UniformConstant %u_storage_1d
    %ptr_u_storage_2d         = OpTypePointer UniformConstant %u_storage_2d
    %ptr_u_storage_2d_array   = OpTypePointer UniformConstant %u_storage_2d_array
    %ptr_u_storage_3d         = OpTypePointer UniformConstant %u_storage_3d

    ; Now all the same, but for signed integer sampled type.

    %ptr_i_texture_1d          = OpTypePointer UniformConstant %i_texture_1d
    %ptr_i_texture_2d          = OpTypePointer UniformConstant %i_texture_2d
    %ptr_i_texture_2d_ms       = OpTypePointer UniformConstant %i_texture_2d_ms
    %ptr_i_texture_2d_array    = OpTypePointer UniformConstant %i_texture_2d_array
    %ptr_i_texture_2d_ms_array = OpTypePointer UniformConstant %i_texture_2d_ms_array
    %ptr_i_texture_3d          = OpTypePointer UniformConstant %i_texture_3d
    %ptr_i_texture_cube        = OpTypePointer UniformConstant %i_texture_cube
    %ptr_i_texture_cube_array  = OpTypePointer UniformConstant %i_texture_cube_array

    %ptr_i_storage_1d         = OpTypePointer UniformConstant %i_storage_1d
    %ptr_i_storage_2d         = OpTypePointer UniformConstant %i_storage_2d
    %ptr_i_storage_2d_array   = OpTypePointer UniformConstant %i_storage_2d_array
    %ptr_i_storage_3d         = OpTypePointer UniformConstant %i_storage_3d

  )";
}

std::string CommonTypes() {
    return CommonBasicTypes() + CommonImageTypes();
}

std::string Bindings(std::vector<uint32_t> ids) {
    StringStream os;
    int binding = 0;
    for (auto id : ids) {
        os << "  OpDecorate %" << id << " DescriptorSet 0\n"
           << "  OpDecorate %" << id << " Binding " << binding++ << "\n";
    }
    return os.str();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_WellFormedButNotAHandle) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %10 = OpConstantNull %ptr_sampler
     %20 = OpConstantNull %ptr_f_texture_1d
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule()) << assembly;
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(10, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(20, true);

    EXPECT_EQ(sampler, nullptr);
    EXPECT_EQ(image, nullptr);
    EXPECT_TRUE(p->error().empty());

    p->DeliberatelyInvalidSpirv();  // WGSL does not have null pointers.
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_Direct) {
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(
     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(10, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(20, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_AccessChain) {
    // Show that we would generalize to arrays of handles, even though that
    // is not supported in WGSL MVP.
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpAccessChain %ptr_sampler %10 %uint_1
     %120 = OpAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // WGSL does not support arrays of textures and samplers.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_InBoundsAccessChain) {
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpInBoundsAccessChain %ptr_sampler %10 %uint_1
     %120 = OpInBoundsAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // WGSL does not support arrays of textures and samplers.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_PtrAccessChain) {
    // Show that we would generalize to arrays of handles, even though that
    // is not supported in WGSL MVP.
    // Use VariablePointers for the OpInBoundsPtrAccessChain.
    const auto assembly = "OpCapability VariablePointers " + Preamble() + FragMain() +
                          Bindings({10, 20}) + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // Variable pointers is not allowed for WGSL. So don't dump it.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_InBoundsPtrAccessChain) {
    // Use VariablePointers for the OpInBoundsPtrAccessChain.
    const auto assembly = "OpCapability VariablePointers " + Preamble() + FragMain() +
                          Bindings({10, 20}) + CommonTypes() + R"(

     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %10 = OpVariable %ptr_sampler_array UniformConstant
     %20 = OpVariable %ptr_image_array UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpInBoundsPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpInBoundsPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // Variable pointers is not allowed for WGSL. So don't dump it.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_CopyObject) {
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpCopyObject %ptr_sampler %10
     %120 = OpCopyObject %ptr_f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_Load) {
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %110 = OpLoad %sampler %10
     %120 = OpLoad %f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_SampledImage) {
    // Trace through the sampled image instruction, but in two different
    // directions.
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(100, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(100, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_Variable_Image) {
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_1d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s
     %200 = OpImage %f_texture_1d %100

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());

    const auto* image = p->GetMemoryObjectDeclarationForHandle(200, true);
    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_Direct) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(10, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(20, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    p->SkipDumpingPending("crbug.com/tint/1039");
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_AccessChain) {
    // Show that we would generalize to arrays of handles, even though that
    // is not supported in WGSL MVP.
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpAccessChain %ptr_sampler %10 %uint_1
     %120 = OpAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // WGSL does not support arrays of textures or samplers
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_InBoundsAccessChain) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpInBoundsAccessChain %ptr_sampler %10 %uint_1
     %120 = OpInBoundsAccessChain %ptr_f_texture_1d %20 %uint_2

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // WGSL does not support arrays of textures or samplers
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_PtrAccessChain) {
    // Show that we would generalize to arrays of handles, even though that
    // is not supported in WGSL MVP.
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // Variable pointers is not allowed for WGSL. So don't dump it.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_InBoundsPtrAccessChain) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %sampler_array = OpTypeArray %sampler %uint_100
     %image_array = OpTypeArray %f_texture_1d %uint_100

     %ptr_sampler_array = OpTypePointer UniformConstant %sampler_array
     %ptr_image_array = OpTypePointer UniformConstant %image_array

     %fty = OpTypeFunction %void %ptr_sampler_array %ptr_image_array

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler_array
     %20 = OpFunctionParameter %ptr_image_array
     %entry = OpLabel

     %110 = OpInBoundsPtrAccessChain %ptr_sampler %10 %uint_1 %uint_1
     %120 = OpInBoundsPtrAccessChain %ptr_f_texture_1d %20 %uint_1 %uint_2

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    // Variable pointers is not allowed for WGSL. So don't dump it.
    p->DeliberatelyInvalidSpirv();
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_CopyObject) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %110 = OpCopyObject %ptr_sampler %10
     %120 = OpCopyObject %ptr_f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    p->SkipDumpingPending("crbug.com/tint/1039");
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_Load) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %110 = OpLoad %sampler %10
     %120 = OpLoad %f_texture_1d %20

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(110, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(120, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    p->SkipDumpingPending("crbug.com/tint/1039");
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_SampledImage) {
    // Trace through the sampled image instruction, but in two different
    // directions.
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());
    const auto* sampler = p->GetMemoryObjectDeclarationForHandle(100, false);
    const auto* image = p->GetMemoryObjectDeclarationForHandle(100, true);

    ASSERT_TRUE(sampler != nullptr);
    EXPECT_EQ(sampler->result_id(), 10u);

    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    p->SkipDumpingPending("crbug.com/tint/1039");
}

TEST_F(SpvParserHandleTest, GetMemoryObjectDeclarationForHandle_FuncParam_Image) {
    const auto assembly = Preamble() + FragMain() + CommonTypes() + R"(
     %sampled_image_type = OpTypeSampledImage %f_texture_1d

     %fty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_1d

     %func = OpFunction %void None %fty
     %10 = OpFunctionParameter %ptr_sampler
     %20 = OpFunctionParameter %ptr_f_texture_1d
     %entry = OpLabel

     %s = OpLoad %sampler %10
     %im = OpLoad %f_texture_1d %20
     %100 = OpSampledImage %sampled_image_type %im %s
     %200 = OpImage %f_texture_1d %100

     OpReturn
     OpFunctionEnd
  )" + MainBody();
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->error().empty());

    const auto* image = p->GetMemoryObjectDeclarationForHandle(200, true);
    ASSERT_TRUE(image != nullptr);
    EXPECT_EQ(image->result_id(), 20u);

    p->SkipDumpingPending("crbug.com/tint/1039");
}

// Test RegisterHandleUsage, sampled image cases

struct UsageImageAccessCase {
    std::string inst;
    std::string expected_sampler_usage;
    std::string expected_image_usage;
};
inline std::ostream& operator<<(std::ostream& out, const UsageImageAccessCase& c) {
    out << "UsageImageAccessCase(" << c.inst << ", " << c.expected_sampler_usage << ", "
        << c.expected_image_usage << ")";
    return out;
}

using SpvParserHandleTest_RegisterHandleUsage_SampledImage =
    SpirvASTParserTestBase<::testing::TestWithParam<UsageImageAccessCase>>;

TEST_P(SpvParserHandleTest_RegisterHandleUsage_SampledImage, Variable) {
    const std::string inst = GetParam().inst;
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(
     %si_ty = OpTypeSampledImage %f_texture_2d
     %coords = OpConstantNull %v2float
     %coords3d = OpConstantNull %v3float ; needed for Proj variants

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_2d UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %sam = OpLoad %sampler %10
     %im = OpLoad %f_texture_2d %20
     %sampled_image = OpSampledImage %si_ty %im %sam
)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->RegisterHandleUsage());
    EXPECT_TRUE(p->error().empty());
    Usage su = p->GetHandleUsage(10);
    Usage iu = p->GetHandleUsage(20);

    EXPECT_THAT(su.to_str(), Eq(GetParam().expected_sampler_usage));
    EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));

    if (inst.find("ImageQueryLod") != std::string::npos) {
        // WGSL does not support querying image level of detail.
        // So don't emit them as part of a "passing" corpus.
        p->DeliberatelyInvalidSpirv();
    }
    if (inst.find("ImageSampleDrefExplicitLod") != std::string::npos) {
        p->SkipDumpingPending("crbug.com/tint/425");  // gpuweb issue #1319
    }
}

TEST_P(SpvParserHandleTest_RegisterHandleUsage_SampledImage, FunctionParam) {
    const std::string inst = GetParam().inst;
    const auto assembly = Preamble() + FragMain() + Bindings({10, 20}) + CommonTypes() + R"(
     %f_ty = OpTypeFunction %void %ptr_sampler %ptr_f_texture_2d
     %si_ty = OpTypeSampledImage %f_texture_2d
     %coords = OpConstantNull %v2float
     %coords3d = OpConstantNull %v3float ; needed for Proj variants
     %component = OpConstant %uint 1

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_f_texture_2d UniformConstant

     %func = OpFunction %void None %f_ty
     %110 = OpFunctionParameter %ptr_sampler
     %120 = OpFunctionParameter %ptr_f_texture_2d
     %func_entry = OpLabel
     %sam = OpLoad %sampler %110
     %im = OpLoad %f_texture_2d %120
     %sampled_image = OpSampledImage %si_ty %im %sam

)" + inst + R"(

     OpReturn
     OpFunctionEnd

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %foo = OpFunctionCall %void %func %10 %20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule()) << p->error() << assembly << "\n";
    EXPECT_TRUE(p->RegisterHandleUsage()) << p->error() << assembly << "\n";
    EXPECT_TRUE(p->error().empty()) << p->error() << assembly << "\n";
    Usage su = p->GetHandleUsage(10);
    Usage iu = p->GetHandleUsage(20);

    EXPECT_THAT(su.to_str(), Eq(GetParam().expected_sampler_usage));
    EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));

    if (inst.find("ImageQueryLod") != std::string::npos) {
        // WGSL does not support querying image level of detail.
        // So don't emit them as part of a "passing" corpus.
        p->DeliberatelyInvalidSpirv();
    }
    p->SkipDumpingPending("crbug.com/tint/785");
}

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserHandleTest_RegisterHandleUsage_SampledImage,
    ::testing::Values(

        // OpImageGather
        UsageImageAccessCase{"%result = OpImageGather "
                             "%v4float %sampled_image %coords %uint_1",
                             "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"},
        // OpImageDrefGather
        UsageImageAccessCase{"%result = OpImageDrefGather "
                             "%v4float %sampled_image %coords %depth",
                             "Usage(Sampler( comparison ))", "Usage(Texture( is_sampled depth ))"},

        // Sample the texture.

        // OpImageSampleImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleImplicitLod "
                             "%v4float %sampled_image %coords",
                             "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"},
        // OpImageSampleExplicitLod
        UsageImageAccessCase{"%result = OpImageSampleExplicitLod "
                             "%v4float %sampled_image %coords Lod %float_null",
                             "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"},
        // OpImageSampleDrefImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleDrefImplicitLod "
                             "%float %sampled_image %coords %depth",
                             "Usage(Sampler( comparison ))", "Usage(Texture( is_sampled depth ))"},
        // OpImageSampleDrefExplicitLod
        UsageImageAccessCase{"%result = OpImageSampleDrefExplicitLod "
                             "%float %sampled_image %coords %depth Lod %float_null",
                             "Usage(Sampler( comparison ))", "Usage(Texture( is_sampled depth ))"},

        // Sample the texture, with *Proj* variants, even though WGSL doesn't
        // support them.

        // OpImageSampleProjImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjImplicitLod "
                             "%v4float %sampled_image %coords3d",
                             "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"},
        // OpImageSampleProjExplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjExplicitLod "
                             "%v4float %sampled_image %coords3d Lod %float_null",
                             "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"},
        // OpImageSampleProjDrefImplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjDrefImplicitLod "
                             "%float %sampled_image %coords3d %depth",
                             "Usage(Sampler( comparison ))", "Usage(Texture( is_sampled depth ))"},
        // OpImageSampleProjDrefExplicitLod
        UsageImageAccessCase{"%result = OpImageSampleProjDrefExplicitLod "
                             "%float %sampled_image %coords3d %depth Lod %float_null",
                             "Usage(Sampler( comparison ))", "Usage(Texture( is_sampled depth ))"},

        // OpImageQueryLod
        UsageImageAccessCase{"%result = OpImageQueryLod %v2float %sampled_image %coords",
                             "Usage(Sampler( ))", "Usage(Texture( is_sampled ))"}));

// Test RegisterHandleUsage, raw image cases.
// For these we test the use of an image value directly, and not combined
// with the sampler. The image still could be of sampled image type.

struct UsageRawImageCase {
    std::string type;  // Example: f_storage_1d or f_texture_1d
    std::string inst;
    std::string expected_image_usage;
};
inline std::ostream& operator<<(std::ostream& out, const UsageRawImageCase& c) {
    out << "UsageRawImageCase(" << c.type << ", " << c.inst << ", " << c.expected_image_usage
        << ")";
    return out;
}

using SpvParserHandleTest_RegisterHandleUsage_RawImage =
    SpirvASTParserTestBase<::testing::TestWithParam<UsageRawImageCase>>;

TEST_P(SpvParserHandleTest_RegisterHandleUsage_RawImage, Variable) {
    const bool is_storage = GetParam().type.find("storage") != std::string::npos;
    const bool is_write = GetParam().inst.find("ImageWrite") != std::string::npos;
    const auto assembly = Preamble() + FragMain() + Bindings({20}) +
                          (is_storage ? std::string("OpDecorate %20 ") +
                                            std::string(is_write ? "NonReadable" : "NonWritable")
                                      : std::string("")) +
                          " " + CommonTypes() + R"(
     %20 = OpVariable %ptr_)" +
                          GetParam().type + R"( UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %im = OpLoad %)" + GetParam().type +
                          R"( %20
)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->RegisterHandleUsage());
    EXPECT_TRUE(p->error().empty());

    Usage iu = p->GetHandleUsage(20);
    EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));

    Usage su = p->GetHandleUsage(20);
}

TEST_P(SpvParserHandleTest_RegisterHandleUsage_RawImage, FunctionParam) {
    const bool is_storage = GetParam().type.find("storage") != std::string::npos;
    const bool is_write = GetParam().inst.find("ImageWrite") != std::string::npos;
    const auto assembly = Preamble() + FragMain() + Bindings({20}) +
                          (is_storage ? std::string("OpDecorate %20 ") +
                                            std::string(is_write ? "NonReadable" : "NonWritable")
                                      : std::string("")) +
                          " " + CommonTypes() + R"(
     %f_ty = OpTypeFunction %void %ptr_)" +
                          GetParam().type + R"(

     %20 = OpVariable %ptr_)" +
                          GetParam().type + R"( UniformConstant

     %func = OpFunction %void None %f_ty
     %i_param = OpFunctionParameter %ptr_)" +
                          GetParam().type + R"(
     %func_entry = OpLabel
     %im = OpLoad %)" + GetParam().type +
                          R"( %i_param

)" + GetParam().inst + R"(

     OpReturn
     OpFunctionEnd

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %foo = OpFunctionCall %void %func %20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildInternalModule());
    EXPECT_TRUE(p->RegisterHandleUsage());
    EXPECT_TRUE(p->error().empty());
    Usage iu = p->GetHandleUsage(20);

    EXPECT_THAT(iu.to_str(), Eq(GetParam().expected_image_usage));

    // Textures and samplers not yet supported as function parameters.
    p->SkipDumpingPending("crbug.com/tint/785");
}

INSTANTIATE_TEST_SUITE_P(
    Samples,
    SpvParserHandleTest_RegisterHandleUsage_RawImage,
    ::testing::Values(

        // OpImageRead
        UsageRawImageCase{"f_storage_1d", "%result = OpImageRead %v4float %im %uint_1",
                          "Usage(Texture( read ))"},

        // OpImageWrite
        UsageRawImageCase{"f_storage_1d", "OpImageWrite %im %uint_1 %v4float_null",
                          "Usage(Texture( write ))"},

        // OpImageFetch
        UsageRawImageCase{"f_texture_1d",
                          "%result = OpImageFetch "
                          "%v4float %im %uint_0",
                          "Usage(Texture( is_sampled ))"},

        // Image queries

        // OpImageQuerySizeLod
        UsageRawImageCase{"f_texture_2d",
                          "%result = OpImageQuerySizeLod "
                          "%v2uint %im %uint_1",
                          "Usage(Texture( is_sampled ))"},

        // OpImageQuerySize
        // Could be MS=1 or storage image. So it's non-committal.
        UsageRawImageCase{"f_storage_2d",
                          "%result = OpImageQuerySize "
                          "%v2uint %im",
                          "Usage()"},

        // OpImageQueryLevels
        UsageRawImageCase{"f_texture_2d",
                          "%result = OpImageQueryLevels "
                          "%uint %im",
                          "Usage(Texture( ))"},

        // OpImageQuerySamples
        UsageRawImageCase{"f_texture_2d_ms",
                          "%result = OpImageQuerySamples "
                          "%uint %im",
                          "Usage(Texture( is_sampled ms ))"}));

// Test emission of handle variables.

// Test emission of variables where we don't have enough clues from their
// use in image access instructions in executable code.  For these we have
// to infer usage from the SPIR-V sampler or image type.
struct DeclUnderspecifiedHandleCase {
    std::string decorations;  // SPIR-V decorations
    std::string inst;         // SPIR-V variable declarations
    std::string var_decl;     // WGSL variable declaration
};
inline std::ostream& operator<<(std::ostream& out, const DeclUnderspecifiedHandleCase& c) {
    out << "DeclUnderspecifiedHandleCase(" << c.inst << "\n" << c.var_decl << ")";
    return out;
}

using SpvParserHandleTest_DeclUnderspecifiedHandle =
    SpirvASTParserTestBase<::testing::TestWithParam<DeclUnderspecifiedHandleCase>>;

TEST_P(SpvParserHandleTest_DeclUnderspecifiedHandle, Variable) {
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
)" + GetParam().decorations +
                          CommonTypes() + GetParam().inst +
                          R"(

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty()) << p->error();
    const auto program = test::ToString(p->program());
    EXPECT_THAT(program, HasSubstr(GetParam().var_decl)) << program;
}

INSTANTIATE_TEST_SUITE_P(Samplers,
                         SpvParserHandleTest_DeclUnderspecifiedHandle,
                         ::testing::Values(

                             DeclUnderspecifiedHandleCase{
                                 "", R"(
         %ptr = OpTypePointer UniformConstant %sampler
         %10 = OpVariable %ptr UniformConstant
)",
                                 R"(@group(0) @binding(0) var x_10 : sampler;)"}));

INSTANTIATE_TEST_SUITE_P(
    Images,
    SpvParserHandleTest_DeclUnderspecifiedHandle,
    ::testing::Values(

        DeclUnderspecifiedHandleCase{"", R"(
         %10 = OpVariable %ptr_f_texture_1d UniformConstant
)",
                                     R"(@group(0) @binding(0) var x_10 : texture_1d<f32>;)"},
        DeclUnderspecifiedHandleCase{
            R"(
         OpDecorate %10 NonWritable
)",
            R"(
         %10 = OpVariable %ptr_f_storage_1d UniformConstant
)",
            R"(@group(0) @binding(0) var x_10 : texture_1d<f32>;)"},
        DeclUnderspecifiedHandleCase{
            R"(
         OpDecorate %10 NonReadable
)",
            R"(
         %10 = OpVariable %ptr_f_storage_1d UniformConstant
)",
            R"(@group(0) @binding(0) var x_10 : texture_storage_1d<rg32float, write>;)"}));

// Test handle declaration or error, when there is an image access.

struct ImageDeclCase {
    // SPIR-V image type, excluding result ID and opcode
    std::string spirv_image_type_details;
    std::string spirv_image_access;  // Optional instruction to provoke use
    std::string expected_error;
    std::string expected_decl;
};

inline std::ostream& operator<<(std::ostream& out, const ImageDeclCase& c) {
    out << "ImageDeclCase(" << c.spirv_image_type_details << "\n"
        << "access: " << c.spirv_image_access << "\n"
        << "error: " << c.expected_error << "\n"
        << "decl:" << c.expected_decl << "\n)";
    return out;
}

using SpvParserHandleTest_ImageDeclTest =
    SpirvASTParserTestBase<::testing::TestWithParam<ImageDeclCase>>;

TEST_P(SpvParserHandleTest_ImageDeclTest, DeclareAndUseHandle) {
    // Only declare the sampled image type, and the associated variable
    // if the requested image type is a sampled image type and not multisampled.
    const bool is_sampled_image_type =
        GetParam().spirv_image_type_details.find("0 1 Unknown") != std::string::npos;
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft
     OpName %float_var "float_var"
     OpName %ptr_float "ptr_float"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
     OpDecorate %30 DescriptorSet 0
     OpDecorate %30 Binding 1
)" + CommonBasicTypes() +
                          R"(
     %sampler = OpTypeSampler
     %ptr_sampler = OpTypePointer UniformConstant %sampler
     %im_ty = OpTypeImage )" +
                          GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty
)" + (is_sampled_image_type ? " %si_ty = OpTypeSampledImage %im_ty " : "") +
                          R"(

     %ptr_float = OpTypePointer Function %float

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_im_ty UniformConstant
     %30 = OpVariable %ptr_sampler UniformConstant ; comparison sampler, when needed

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %float_var = OpVariable %ptr_float Function

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %sam = OpLoad %sampler %10
     %im = OpLoad %im_ty %20

)" + (is_sampled_image_type ? " %sampled_image = OpSampledImage %si_ty %im %sam " : "") +
                          GetParam().spirv_image_access +
                          R"(
     ; Use an anchor for the cases when the image access doesn't have a result ID.
     %1000 = OpCopyObject %uint %uint_0

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    const bool succeeded = p->BuildAndParseInternalModule();
    if (succeeded) {
        EXPECT_TRUE(GetParam().expected_error.empty());
        const auto got = test::ToString(p->program());
        EXPECT_THAT(got, HasSubstr(GetParam().expected_decl));
    } else {
        EXPECT_FALSE(GetParam().expected_error.empty());
        EXPECT_THAT(p->error(), HasSubstr(GetParam().expected_error));
    }
}

INSTANTIATE_TEST_SUITE_P(
    Multisampled_Only2DNonArrayedIsValid,
    SpvParserHandleTest_ImageDeclTest,
    ::testing::ValuesIn(std::vector<ImageDeclCase>{
        {"%float 1D 0 0 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL multisampled textures must be 2d and non-arrayed: ", ""},
        {"%float 1D 0 1 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL arrayed textures must be 2d_array or cube_array: ", ""},
        {"%float 2D 0 0 1 1 Unknown", "%result = OpImageQuerySamples %uint %im", "",
         "@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;"},
        {"%float 2D 0 1 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL multisampled textures must be 2d and non-arrayed: ", ""},
        {"%float 3D 0 0 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL multisampled textures must be 2d and non-arrayed: ", ""},
        {"%float 3D 0 1 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL arrayed textures must be 2d_array or cube_array: ", ""},
        {"%float Cube 0 0 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL multisampled textures must be 2d and non-arrayed: ", ""},
        {"%float Cube 0 1 1 1 Unknown", "%result = OpImageQuerySamples %uint %im",
         "WGSL multisampled textures must be 2d and non-arrayed: ", ""}}));

// Test emission of variables when we have image accesses in executable code.

struct ImageAccessCase {
    // SPIR-V image type, excluding result ID and opcode
    std::string spirv_image_type_details;
    std::string spirv_image_access;  // The provoking image access instruction.
    std::string var_decl;            // WGSL variable declaration
    std::string texture_builtin;     // WGSL texture usage.
};
inline std::ostream& operator<<(std::ostream& out, const ImageAccessCase& c) {
    out << "ImageCase(" << c.spirv_image_type_details << "\n"
        << c.spirv_image_access << "\n"
        << c.var_decl << "\n"
        << c.texture_builtin << ")";
    return out;
}

using SpvParserHandleTest_SampledImageAccessTest =
    SpirvASTParserTestBase<::testing::TestWithParam<ImageAccessCase>>;

TEST_P(SpvParserHandleTest_SampledImageAccessTest, Variable) {
    // Only declare the sampled image type, and the associated variable
    // if the requested image type is a sampled image type, and not a
    // multisampled texture
    const bool is_sampled_image_type =
        GetParam().spirv_image_type_details.find("0 1 Unknown") != std::string::npos;
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf21 "vf21"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %coords1 "coords1"
     OpName %coords12 "coords12"
     OpName %coords123 "coords123"
     OpName %coords1234 "coords1234"
     OpName %offsets2d "offsets2d"
     OpName %u_offsets2d "u_offsets2d"
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
     OpDecorate %30 DescriptorSet 0
     OpDecorate %30 Binding 1
)" + CommonBasicTypes() +
                          R"(
     %sampler = OpTypeSampler
     %ptr_sampler = OpTypePointer UniformConstant %sampler
     %im_ty = OpTypeImage )" +
                          GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty
)" + (is_sampled_image_type ? " %si_ty = OpTypeSampledImage %im_ty " : "") +
                          R"(

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_im_ty UniformConstant
     %30 = OpVariable %ptr_sampler UniformConstant ; comparison sampler, when needed

     ; ConstOffset operands must be constants
     %offsets2d = OpConstantComposite %v2int %int_3 %int_4
     %u_offsets2d = OpConstantComposite %v2uint %uint_3 %uint_4

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf21 = OpCopyObject %v2float %the_vf21
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %coords1 = OpCopyObject %float %float_1
     %coords12 = OpCopyObject %v2float %vf12
     %coords123 = OpCopyObject %v3float %vf123
     %coords1234 = OpCopyObject %v4float %vf1234

     %sam = OpLoad %sampler %10
     %im = OpLoad %im_ty %20
)" + (is_sampled_image_type ? " %sampled_image = OpSampledImage %si_ty %im %sam\n" : "") +
                          GetParam().spirv_image_access +
                          R"(

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty()) << p->error();
    const auto program = test::ToString(p->program());
    EXPECT_THAT(program, HasSubstr(GetParam().var_decl)) << "DECLARATIONS ARE BAD " << program;
    EXPECT_THAT(program, HasSubstr(GetParam().texture_builtin))
        << "TEXTURE BUILTIN IS BAD " << program << assembly;

    const bool is_query_size =
        GetParam().spirv_image_access.find("ImageQuerySize") != std::string::npos;
    const bool is_1d = GetParam().spirv_image_type_details.find("1D") != std::string::npos;
    if (is_query_size && is_1d) {
        p->SkipDumpingPending("crbug.com/tint/788");
    }
}

INSTANTIATE_TEST_SUITE_P(
    ImageGather,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // OpImageGather 2D
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords12 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        "textureGather(1i, x_20, x_10, coords12)"},
        // OpImageGather 2D ConstOffset signed
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords12 %int_1 ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        "textureGather(1i, x_20, x_10, coords12, offsets2d)"},
        // OpImageGather 2D ConstOffset unsigned
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords12 %int_1 ConstOffset "
                        "%u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        "textureGather(1i, x_20, x_10, coords12, "
                        "vec2i(u_offsets2d))"},
        // OpImageGather 2D Array
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
                        "textureGather(1i, x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)))"},
        // OpImageGather 2D Array ConstOffset signed
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1 ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
                        "textureGather(1i, x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), offsets2d)"},
        // OpImageGather 2D Array ConstOffset unsigned
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1 ConstOffset "
                        "%u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
                        "textureGather(1i, x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), "
                        "vec2i(u_offsets2d))"},
        // OpImageGather Cube
        ImageAccessCase{"%float Cube 0 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_cube<f32>;)",
                        "textureGather(1i, x_20, x_10, coords123)"},
        // OpImageGather Cube Array
        ImageAccessCase{"%float Cube 0 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords1234 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_cube_array<f32>;)",
                        "textureGather(1i, x_20, x_10, coords1234.xyz, "
                        "i32(round(coords1234.w)))"},
        // OpImageGather 2DDepth
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords12 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                        "textureGather(x_20, x_10, coords12)"},
        // OpImageGather 2DDepth ConstOffset signed
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords12 %int_1 ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                        "textureGather(x_20, x_10, coords12, offsets2d)"},
        // OpImageGather 2DDepth ConstOffset unsigned
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords12 %int_1 ConstOffset "
                        "%u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                        "textureGather(x_20, x_10, coords12, "
                        "vec2i(u_offsets2d))"},
        // OpImageGather 2DDepth Array
        ImageAccessCase{"%float 2D 1 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                        "textureGather(x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)))"},
        // OpImageGather 2DDepth Array ConstOffset signed
        ImageAccessCase{"%float 2D 1 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1 ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                        "textureGather(x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), offsets2d)"},
        // OpImageGather 2DDepth Array ConstOffset unsigned
        ImageAccessCase{"%float 2D 1 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1 ConstOffset "
                        "%u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                        "textureGather(x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), "
                        "vec2i(u_offsets2d))"},
        // OpImageGather DepthCube
        ImageAccessCase{"%float Cube 1 0 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords123 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_cube;)",
                        "textureGather(x_20, x_10, coords123)"},
        // OpImageGather DepthCube Array
        ImageAccessCase{"%float Cube 1 1 0 1 Unknown",
                        "%result = OpImageGather "
                        "%v4float %sampled_image %coords1234 %int_1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_cube_array;)",
                        "textureGather(x_20, x_10, coords1234.xyz, "
                        "i32(round(coords1234.w)))"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageDrefGather,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // OpImageDrefGather 2DDepth
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords12 %depth",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                        "textureGatherCompare(x_20, x_10, coords12, 0.20000000298023223877f)"},
        // OpImageDrefGather 2DDepth ConstOffset signed
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords12 %depth ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                        "textureGatherCompare(x_20, x_10, coords12, 0.20000000298023223877f, "
                        "offsets2d)"},
        // OpImageDrefGather 2DDepth ConstOffset unsigned
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords12 %depth ConstOffset "
                        "%u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                        "textureGatherCompare(x_20, x_10, coords12, 0.20000000298023223877f, "
                        "vec2i(u_offsets2d))"},
        // OpImageDrefGather 2DDepth Array
        ImageAccessCase{"%float 2D 1 1 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords123 %depth",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                        "textureGatherCompare(x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), 0.20000000298023223877f)"},
        // OpImageDrefGather 2DDepth Array ConstOffset signed
        ImageAccessCase{"%float 2D 1 1 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords123 %depth ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                        "textureGatherCompare(x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), 0.20000000298023223877f, offsets2d)"},
        // OpImageDrefGather 2DDepth Array ConstOffset unsigned
        ImageAccessCase{"%float 2D 1 1 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords123 %depth ConstOffset "
                        "%u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                        "textureGatherCompare(x_20, x_10, coords123.xy, "
                        "i32(round(coords123.z)), 0.20000000298023223877f, "
                        "vec2i(u_offsets2d))"},
        // OpImageDrefGather DepthCube
        ImageAccessCase{"%float Cube 1 0 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords123 %depth",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_cube;)",
                        "textureGatherCompare(x_20, x_10, coords123, 0.20000000298023223877f)"},
        // OpImageDrefGather DepthCube Array
        ImageAccessCase{"%float Cube 1 1 0 1 Unknown",
                        "%result = OpImageDrefGather "
                        "%v4float %sampled_image %coords1234 %depth",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_cube_array;)",
                        "textureGatherCompare(x_20, x_10, coords1234.xyz, "
                        "i32(round(coords1234.w)), 0.20000000298023223877f)"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleImplicitLod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleImplicitLod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        "textureSample(x_20, x_10, coords12)"},

        // OpImageSampleImplicitLod arrayed
        ImageAccessCase{"%float 2D 0 1 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords123",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
                        "textureSample(x_20, x_10, coords123.xy, i32(round(coords123.z)))"},

        // OpImageSampleImplicitLod with ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        "textureSample(x_20, x_10, coords12, offsets2d)"},

        // OpImageSampleImplicitLod arrayed with ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleImplicitLod "
            "%v4float %sampled_image %coords123 ConstOffset %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSample(x_20, x_10, coords123.xy, i32(round(coords123.z)), offsets2d))"},

        // OpImageSampleImplicitLod with Bias
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 Bias %float_7",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        "textureSampleBias(x_20, x_10, coords12, 7.0f)"},

        // OpImageSampleImplicitLod arrayed with Bias
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleImplicitLod "
            "%v4float %sampled_image %coords123 Bias %float_7",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleBias(x_20, x_10, coords123.xy, i32(round(coords123.z)), 7.0f))"},

        // OpImageSampleImplicitLod with Bias and signed ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 Bias|ConstOffset "
                        "%float_7 %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleBias(x_20, x_10, coords12, 7.0f, offsets2d)"},

        // OpImageSampleImplicitLod with Bias and unsigned ConstOffset
        // Convert ConstOffset to signed
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleImplicitLod "
                        "%v4float %sampled_image %coords12 Bias|ConstOffset "
                        "%float_7 %u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleBias(x_20, x_10, coords12, 7.0f, vec2i(u_offsets2d))"},
        // OpImageSampleImplicitLod arrayed with Bias
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleImplicitLod "
            "%v4float %sampled_image %coords123 Bias|ConstOffset "
            "%float_7 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleBias(x_20, x_10, coords123.xy, i32(round(coords123.z)), 7.0f, offsets2d)"}));

INSTANTIATE_TEST_SUITE_P(
    // This test shows the use of a sampled image used with both regular
    // sampling and depth-reference sampling.  The texture is a depth-texture,
    // and we use builtins textureSample and textureSampleCompare
    ImageSampleImplicitLod_BothDrefAndNonDref,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleImplicitLod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown", R"(
     %sam_dref = OpLoad %sampler %30
     %sampled_dref_image = OpSampledImage %si_ty %im %sam_dref

     %200 = OpImageSampleImplicitLod %v4float %sampled_image %coords12
     %210 = OpImageSampleDrefImplicitLod %float %sampled_dref_image %coords12 %depth
)",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d;

@group(0) @binding(1) var x_30 : sampler_comparison;
)",
                        R"(

@group(2) @binding(1) var x_20 : texture_depth_2d;

@group(0) @binding(1) var x_30 : sampler_comparison;

fn main_1() {
  let f1 = 1.0f;
  let vf12 = vec2f(1.0f, 2.0f);
  let vf21 = vec2f(2.0f, 1.0f);
  let vf123 = vec3f(1.0f, 2.0f, 3.0f);
  let vf1234 = vec4f(1.0f, 2.0f, 3.0f, 4.0f);
  let i1 = 1i;
  let vi12 = vec2i(1i, 2i);
  let vi123 = vec3i(1i, 2i, 3i);
  let vi1234 = vec4i(1i, 2i, 3i, 4i);
  let u1 = 1u;
  let vu12 = vec2u(1u, 2u);
  let vu123 = vec3u(1u, 2u, 3u);
  let vu1234 = vec4u(1u, 2u, 3u, 4u);
  let coords1 = 1.0f;
  let coords12 = vf12;
  let coords123 = vf123;
  let coords1234 = vf1234;
  let x_200 = vec4f(textureSample(x_20, x_10, coords12), 0.0f, 0.0f, 0.0f);
  let x_210 = textureSampleCompare(x_20, x_30, coords12, 0.20000000298023223877f);
)"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDrefImplicitLod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(
        // ImageSampleDrefImplicitLod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleDrefImplicitLod "
                        "%float %sampled_image %coords12 %depth",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
                        R"(textureSampleCompare(x_20, x_10, coords12, 0.20000000298023223877f))"},
        // ImageSampleDrefImplicitLod - arrayed
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleDrefImplicitLod "
            "%float %sampled_image %coords123 %depth",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
            R"(textureSampleCompare(x_20, x_10, coords123.xy, i32(round(coords123.z)), 0.20000000298023223877f))"},
        // ImageSampleDrefImplicitLod with ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleDrefImplicitLod %float "
            "%sampled_image %coords12 %depth ConstOffset %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            R"(textureSampleCompare(x_20, x_10, coords12, 0.20000000298023223877f, offsets2d))"},
        // ImageSampleDrefImplicitLod arrayed with ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleDrefImplicitLod %float "
            "%sampled_image %coords123 %depth ConstOffset %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
            R"(textureSampleCompare(x_20, x_10, coords123.xy, i32(round(coords123.z)), 0.20000000298023223877f, offsets2d))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDrefExplicitLod,
    SpvParserHandleTest_SampledImageAccessTest,
    // Lod must be float constant 0 due to a Metal constraint.
    // Another test checks cases where the Lod is not float constant 0.
    ::testing::Values(
        // 2D
        ImageAccessCase{
            "%float 2D 1 0 0 1 Unknown",
            "%result = OpImageSampleDrefExplicitLod "
            "%float %sampled_image %coords12 %depth Lod %float_0",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            R"(textureSampleCompareLevel(x_20, x_10, coords12, 0.20000000298023223877f))"},
        // 2D array
        ImageAccessCase{
            "%float 2D 1 1 0 1 Unknown",
            "%result = OpImageSampleDrefExplicitLod "
            "%float %sampled_image %coords123 %depth Lod %float_0",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
            R"(textureSampleCompareLevel(x_20, x_10, coords123.xy, i32(round(coords123.z)), 0.20000000298023223877f))"},
        // 2D, ConstOffset
        ImageAccessCase{
            "%float 2D 1 0 0 1 Unknown",
            "%result = OpImageSampleDrefExplicitLod %float "
            "%sampled_image %coords12 %depth Lod|ConstOffset "
            "%float_0 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            R"(textureSampleCompareLevel(x_20, x_10, coords12, 0.20000000298023223877f, offsets2d))"},
        // 2D array, ConstOffset
        ImageAccessCase{
            "%float 2D 1 1 0 1 Unknown",
            "%result = OpImageSampleDrefExplicitLod %float "
            "%sampled_image %coords123 %depth Lod|ConstOffset "
            "%float_0 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
            R"(textureSampleCompareLevel(x_20, x_10, coords123.xy, i32(round(coords123.z)), 0.20000000298023223877f, offsets2d))"},
        // Cube
        ImageAccessCase{
            "%float Cube 1 0 0 1 Unknown",
            "%result = OpImageSampleDrefExplicitLod "
            "%float %sampled_image %coords123 %depth Lod %float_0",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_cube;)",
            R"(textureSampleCompareLevel(x_20, x_10, coords123, 0.20000000298023223877f))"},
        // Cube array
        ImageAccessCase{
            "%float Cube 1 1 0 1 Unknown",
            "%result = OpImageSampleDrefExplicitLod "
            "%float %sampled_image %coords1234 %depth Lod %float_0",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_cube_array;)",
            R"(textureSampleCompareLevel(x_20, x_10, coords1234.xyz, i32(round(coords1234.w)), 0.20000000298023223877f))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleExplicitLod_UsingLod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleExplicitLod - using Lod
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Lod %float_null",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleLevel(x_20, x_10, coords12, 0.0f))"},

        // OpImageSampleExplicitLod arrayed - using Lod
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords123 Lod %float_null",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleLevel(x_20, x_10, coords123.xy, i32(round(coords123.z)), 0.0f))"},

        // OpImageSampleExplicitLod - using Lod and ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Lod|ConstOffset "
                        "%float_null %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleLevel(x_20, x_10, coords12, 0.0f, offsets2d))"},

        // OpImageSampleExplicitLod - using Lod and unsigned ConstOffset
        // Convert the ConstOffset operand to signed
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Lod|ConstOffset "
                        "%float_null %u_offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleLevel(x_20, x_10, coords12, 0.0f, vec2i(u_offsets2d))"},

        // OpImageSampleExplicitLod arrayed - using Lod and ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords123 Lod|ConstOffset "
            "%float_null %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleLevel(x_20, x_10, coords123.xy, i32(round(coords123.z)), 0.0f, offsets2d))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleExplicitLod_UsingGrad,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleExplicitLod - using Grad
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Grad %vf12 %vf21",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleGrad(x_20, x_10, coords12, vf12, vf21))"},

        // OpImageSampleExplicitLod arrayed - using Grad
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords123 Grad %vf12 %vf21",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleGrad(x_20, x_10, coords123.xy, i32(round(coords123.z)), vf12, vf21))"},

        // OpImageSampleExplicitLod - using Grad and ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleExplicitLod "
                        "%v4float %sampled_image %coords12 Grad|ConstOffset "
                        "%vf12 %vf21 %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleGrad(x_20, x_10, coords12, vf12, vf21, offsets2d))"},

        // OpImageSampleExplicitLod - using Grad and unsigned ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords12 Grad|ConstOffset "
            "%vf12 %vf21 %u_offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
            R"(textureSampleGrad(x_20, x_10, coords12, vf12, vf21, vec2i(u_offsets2d))"},

        // OpImageSampleExplicitLod arrayed - using Grad and ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords123 Grad|ConstOffset "
            "%vf12 %vf21 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleGrad(x_20, x_10, coords123.xy, i32(round(coords123.z)), vf12, vf21, offsets2d))"},

        // OpImageSampleExplicitLod arrayed - using Grad and unsigned
        // ConstOffset
        ImageAccessCase{
            "%float 2D 0 1 0 1 Unknown",
            "%result = OpImageSampleExplicitLod "
            "%v4float %sampled_image %coords123 Grad|ConstOffset "
            "%vf12 %vf21 %u_offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
            R"(textureSampleGrad(x_20, x_10, coords123.xy, i32(round(coords123.z)), vf12, vf21, vec2i(u_offsets2d)))"}));

// Test crbug.com/378:
// In WGSL, sampling from depth texture with explicit level of detail
// requires the Lod parameter as an unsigned integer.
// This corresponds to SPIR-V OpSampleExplicitLod and WGSL textureSampleLevel.
INSTANTIATE_TEST_SUITE_P(
    ImageSampleExplicitLod_DepthTexture,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Test a non-depth case.
        // (This is already tested above in the ImageSampleExplicitLod suite,
        // but I'm repeating here for the contrast with the depth case.)
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float "
         "%sampled_image %vf12 Lod %f1",
         R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
         R"(textureSampleLevel(x_20, x_10, vf12, f1))"},
        // Test a depth case
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float "
         "%sampled_image %vf12 Lod %f1",
         R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
         R"(vec4f(textureSampleLevel(x_20, x_10, vf12, i32(f1)), 0.0f, 0.0f, 0.0f))"}}));

/////
// Projection sampling
/////

// Test matrix for projection sampling:
// sampling
//   Dimensions: 1D, 2D, 3D, 2DShadow
//   Variations: Proj { ImplicitLod { | Bias } | ExplicitLod { Lod | Grad | } }
//   x { | ConstOffset }
// depth-compare sampling
//   Dimensions: 2D
//   Variations: Proj Dref { ImplicitLod { | Bias } | ExplicitLod { Lod | Grad |
//   } } x { | ConstOffset }
//
// Expanded:
//    ImageSampleProjImplicitLod        // { | ConstOffset }
//    ImageSampleProjImplicitLod_Bias   // { | ConstOffset }
//    ImageSampleProjExplicitLod_Lod    // { | ConstOffset }
//    ImageSampleProjExplicitLod_Grad   // { | ConstOffset }
//
//    ImageSampleProjImplicitLod_DepthTexture
//
//    ImageSampleProjDrefImplicitLod        // { | ConstOffset }
//    ImageSampleProjDrefExplicitLod_Lod    // { | ConstOffset }

INSTANTIATE_TEST_SUITE_P(
    ImageSampleProjImplicitLod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleProjImplicitLod 1D
        ImageAccessCase{"%float 1D 0 0 0 1 Unknown",
                        "%result = OpImageSampleProjImplicitLod "
                        "%v4float %sampled_image %coords12",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_1d<f32>;)",
                        R"(textureSample(x_20, x_10, (coords12.x / coords12.y)))"},

        // OpImageSampleProjImplicitLod 2D
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleProjImplicitLod "
                        "%v4float %sampled_image %coords123",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSample(x_20, x_10, (coords123.xy / coords123.z)))"},

        // OpImageSampleProjImplicitLod 3D
        ImageAccessCase{"%float 3D 0 0 0 1 Unknown",
                        "%result = OpImageSampleProjImplicitLod "
                        "%v4float %sampled_image %coords1234",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_3d<f32>;)",
                        R"(textureSample(x_20, x_10, (coords1234.xyz / coords1234.w)))"},

        // OpImageSampleProjImplicitLod 2D with ConstOffset
        // (Don't need to test with 1D or 3D, as the hard part was the splatted
        // division.) This case tests handling of the ConstOffset
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleProjImplicitLod "
                        "%v4float %sampled_image %coords123 ConstOffset %offsets2d",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSample(x_20, x_10, (coords123.xy / coords123.z), offsets2d))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleProjImplicitLod_Bias,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleProjImplicitLod with Bias
        // Only testing 2D
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleProjImplicitLod "
                        "%v4float %sampled_image %coords123 Bias %float_7",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleBias(x_20, x_10, (coords123.xy / coords123.z), 7.0f))"},

        // OpImageSampleProjImplicitLod with Bias and signed ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleProjImplicitLod "
            "%v4float %sampled_image %coords123 Bias|ConstOffset "
            "%float_7 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
            R"(textureSampleBias(x_20, x_10, (coords123.xy / coords123.z), 7.0f, offsets2d))"},

        // OpImageSampleProjImplicitLod with Bias and unsigned ConstOffset
        // Convert ConstOffset to signed
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleProjImplicitLod "
            "%v4float %sampled_image %coords123 Bias|ConstOffset "
            "%float_7 %u_offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
            R"(textureSampleBias(x_20, x_10, (coords123.xy / coords123.z), 7.0f, vec2i(u_offsets2d)))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleProjExplicitLod_Lod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(
        // OpImageSampleProjExplicitLod 2D
        ImageAccessCase{"%float 2D 0 0 0 1 Unknown",
                        "%result = OpImageSampleProjExplicitLod "
                        "%v4float %sampled_image %coords123 Lod %f1",
                        R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                        R"(textureSampleLevel(x_20, x_10, (coords123.xy / coords123.z), f1))"},

        // OpImageSampleProjExplicitLod 2D Lod with ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleProjExplicitLod "
            "%v4float %sampled_image %coords123 Lod|ConstOffset %f1 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
            R"(textureSampleLevel(x_20, x_10, (coords123.xy / coords123.z), f1, offsets2d))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleProjExplicitLod_Grad,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(
        // OpImageSampleProjExplicitLod 2D Grad
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleProjExplicitLod "
            "%v4float %sampled_image %coords123 Grad %vf12 %vf21",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
            R"(textureSampleGrad(x_20, x_10, (coords123.xy / coords123.z), vf12, vf21))"},

        // OpImageSampleProjExplicitLod 2D Lod Grad ConstOffset
        ImageAccessCase{
            "%float 2D 0 0 0 1 Unknown",
            "%result = OpImageSampleProjExplicitLod "
            "%v4float %sampled_image %coords123 Grad|ConstOffset "
            "%vf12 %vf21 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
            R"(textureSampleGrad(x_20, x_10, (coords123.xy / coords123.z), vf12, vf21, offsets2d))"}));

INSTANTIATE_TEST_SUITE_P(
    // Ordinary (non-comparison) sampling on a depth texture.
    ImageSampleProjImplicitLod_DepthTexture,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(
        // OpImageSampleProjImplicitLod 2D depth
        ImageAccessCase{
            "%float 2D 1 0 0 1 Unknown",
            "%result = OpImageSampleProjImplicitLod "
            "%v4float %sampled_image %coords123",
            R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            // Sampling the depth texture yields an f32, but the
            // SPIR-V operation yiedls vec4f, so fill out the
            // remaining components with 0.
            R"(vec4f(textureSample(x_20, x_10, (coords123.xy / coords123.z)), 0.0f, 0.0f, 0.0f))"}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleProjDrefImplicitLod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // OpImageSampleProjDrefImplicitLod 2D depth-texture
        ImageAccessCase{"%float 2D 1 0 0 1 Unknown",
                        "%result = OpImageSampleProjDrefImplicitLod "
                        "%float %sampled_image %coords123 %f1",
                        R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
                        R"(textureSampleCompare(x_20, x_10, (coords123.xy / coords123.z), f1))"},

        // OpImageSampleProjDrefImplicitLod 2D depth-texture, ConstOffset
        ImageAccessCase{
            "%float 2D 1 0 0 1 Unknown",
            "%result = OpImageSampleProjDrefImplicitLod "
            "%float %sampled_image %coords123 %f1 ConstOffset %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            R"(textureSampleCompare(x_20, x_10, (coords123.xy / coords123.z), f1, offsets2d))"}));

INSTANTIATE_TEST_SUITE_P(
    DISABLED_ImageSampleProjDrefExplicitLod_Lod,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::Values(

        // Lod must be float constant 0 due to a Metal constraint.
        // Another test checks cases where the Lod is not float constant 0.

        // OpImageSampleProjDrefExplicitLod 2D depth-texture Lod
        ImageAccessCase{
            "%float 2D 1 0 0 1 Unknown",
            "%result = OpImageSampleProjDrefExplicitLod "
            "%float %sampled_image %coords123 %depth Lod %float_0",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            R"(textureSampleCompare(x_20, x_10, (coords123.xy / coords123.z), 0.20000000298023223877f, 0.0f))"},

        // OpImageSampleProjDrefImplicitLod 2D depth-texture, Lod ConstOffset
        ImageAccessCase{
            "%float 2D 1 0 0 1 Unknown",
            "%result = OpImageSampleProjDrefExplicitLod "
            "%float %sampled_image %coords123 %depth "
            "Lod|ConstOffset %float_0 %offsets2d",
            R"(@group(0) @binding(0) var x_10 : sampler_comparison;

@group(2) @binding(1) var x_20 : texture_depth_2d;
)",
            R"(textureSampleCompareLevel(x_20, x_10, (coords123.xy / coords123.z), 0.20000000298023223877f, 0.0f, vec2i(3i, 4i)))"}));

/////
// End projection sampling
/////

using SpvParserHandleTest_ImageAccessTest =
    SpirvASTParserTestBase<::testing::TestWithParam<ImageAccessCase>>;

TEST_P(SpvParserHandleTest_ImageAccessTest, Variable) {
    // In this test harness, we only create an image.
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %offsets2d "offsets2d"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                          R"(
     %im_ty = OpTypeImage )" +
                          GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty
     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %value_offset = OpCompositeConstruct %v2int %int_3 %int_4
     %offsets2d = OpCopyObject %v2int %value_offset
     %im = OpLoad %im_ty %20

)" + GetParam().spirv_image_access +
                          R"(
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    ASSERT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;
    EXPECT_TRUE(p->error().empty()) << p->error();
    const auto program = test::ToString(p->program());
    EXPECT_THAT(program, HasSubstr(GetParam().var_decl)) << "DECLARATIONS ARE BAD " << program;
    EXPECT_THAT(program, HasSubstr(GetParam().texture_builtin))
        << "TEXTURE BUILTIN IS BAD " << program << assembly;
}

INSTANTIATE_TEST_SUITE_P(ImageWrite_OptionalParams,
                         SpvParserHandleTest_ImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // OpImageWrite with no extra params
                             {"%float 2D 0 0 0 2 Rgba32f", "OpImageWrite %im %vi12 %vf1234",
                              "@group(2) @binding(1) var x_20 : "
                              "texture_storage_2d<rgba32float, write>;",
                              "textureStore(x_20, vi12, vf1234);"}}));

INSTANTIATE_TEST_SUITE_P(
    // SPIR-V's texel parameter is a scalar or vector with at least as many
    // components as there are channels in the underlying format, and the
    // component type matches the sampled type (modulo signed/unsigned integer).
    // WGSL's texel parameter is a 4-element vector scalar or vector, with
    // component type equal to the 32-bit form of the channel type.
    ImageWrite_ConvertTexelOperand_Arity_Float,
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Source 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %f1",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32float, write>;)",
         "textureStore(x_20, vi12, vec4f(f1, 0.0f, 0.0f, 0.0f));"},
        // Source 2 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32float, write>;)",
         "textureStore(x_20, vi12, vec4f(vf12, 0.0f, 0.0f));"},
        // Source 3 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf123",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32float, write>;)",
         "textureStore(x_20, vi12, vec4f(vf123, 0.0f));"},
        // Source 4 component, dest 1 component
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32float, write>;)",
         "textureStore(x_20, vi12, vf1234);"},
        // Source 2 component, dest 2 component
        {"%float 2D 0 0 0 2 Rg32f", "OpImageWrite %im %vi12 %vf12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32float, write>;)",
         "textureStore(x_20, vi12, vec4f(vf12, 0.0f, 0.0f));"},
        // Source 3 component, dest 2 component
        {"%float 2D 0 0 0 2 Rg32f", "OpImageWrite %im %vi12 %vf123",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32float, write>;)",
         "textureStore(x_20, vi12, vec4f(vf123, 0.0f));"},
        // Source 4 component, dest 2 component
        {"%float 2D 0 0 0 2 Rg32f", "OpImageWrite %im %vi12 %vf1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32float, write>;)",
         "textureStore(x_20, vi12, vf1234);"},
        // WGSL does not support 3-component storage textures.
        // Source 4 component, dest 4 component
        {"%float 2D 0 0 0 2 Rgba32f", "OpImageWrite %im %vi12 %vf1234",
         "@group(2) @binding(1) var x_20 : "
         "texture_storage_2d<rgba32float, write>;",
         "textureStore(x_20, vi12, vf1234);"}}));

INSTANTIATE_TEST_SUITE_P(
    // As above, but unsigned integer.
    ImageWrite_ConvertTexelOperand_Arity_Uint,
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Source 1 component
        {"%uint 2D 0 0 0 2 R32ui", "OpImageWrite %im %vi12 %u1",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32uint, write>;)",
         "textureStore(x_20, vi12, vec4u(u1, 0u, 0u, 0u));"},
        // Source 2 component, dest 1 component
        {"%uint 2D 0 0 0 2 R32ui", "OpImageWrite %im %vi12 %vu12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32uint, write>;)",
         "textureStore(x_20, vi12, vec4u(vu12, 0u, 0u));"},
        // Source 3 component, dest 1 component
        {"%uint 2D 0 0 0 2 R32ui", "OpImageWrite %im %vi12 %vu123",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32uint, write>;)",
         "textureStore(x_20, vi12, vec4u(vu123, 0u));"},
        // Source 4 component, dest 1 component
        {"%uint 2D 0 0 0 2 R32ui", "OpImageWrite %im %vi12 %vu1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32uint, write>;)",
         "textureStore(x_20, vi12, vu1234);"},
        // Source 2 component, dest 2 component
        {"%uint 2D 0 0 0 2 Rg32ui", "OpImageWrite %im %vi12 %vu12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32uint, write>;)",
         "textureStore(x_20, vi12, vec4u(vu12, 0u, 0u));"},
        // Source 3 component, dest 2 component
        {"%uint 2D 0 0 0 2 Rg32ui", "OpImageWrite %im %vi12 %vu123",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32uint, write>;)",
         "textureStore(x_20, vi12, vec4u(vu123, 0u));"},
        // Source 4 component, dest 2 component
        {"%uint 2D 0 0 0 2 Rg32ui", "OpImageWrite %im %vi12 %vu1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32uint, write>;)",
         "textureStore(x_20, vi12, vu1234);"},
        // WGSL does not support 3-component storage textures.
        // Source 4 component, dest 4 component
        {"%uint 2D 0 0 0 2 Rgba32ui", "OpImageWrite %im %vi12 %vu1234",
         "@group(2) @binding(1) var x_20 : "
         "texture_storage_2d<rgba32uint, write>;",
         "textureStore(x_20, vi12, vu1234);"}}));

INSTANTIATE_TEST_SUITE_P(
    // As above, but signed integer.
    ImageWrite_ConvertTexelOperand_Arity_Sint,
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Source 1 component
        {"%int 2D 0 0 0 2 R32i", "OpImageWrite %im %vi12 %i1",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32sint, write>;)",
         "textureStore(x_20, vi12, vec4i(i1, 0i, 0i, 0i));"},
        // Source 2 component, dest 1 component
        {"%int 2D 0 0 0 2 R32i", "OpImageWrite %im %vi12 %vi12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32sint, write>;)",
         "textureStore(x_20, vi12, vec4i(vi12, 0i, 0i));"},
        // Source 3 component, dest 1 component
        {"%int 2D 0 0 0 2 R32i", "OpImageWrite %im %vi12 %vi123",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32sint, write>;)",
         "textureStore(x_20, vi12, vec4i(vi123, 0i));"},
        // Source 4 component, dest 1 component
        {"%int 2D 0 0 0 2 R32i", "OpImageWrite %im %vi12 %vi1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32sint, write>;)",
         "textureStore(x_20, vi12, vi1234);"},
        // Source 2 component, dest 2 component
        {"%int 2D 0 0 0 2 Rg32i", "OpImageWrite %im %vi12 %vi12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32sint, write>;)",
         "textureStore(x_20, vi12, vec4i(vi12, 0i, 0i));"},
        // Source 3 component, dest 2 component
        {"%int 2D 0 0 0 2 Rg32i", "OpImageWrite %im %vi12 %vi123",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32sint, write>;)",
         "textureStore(x_20, vi12, vec4i(vi123, 0i));"},
        // Source 4 component, dest 2 component
        {"%int 2D 0 0 0 2 Rg32i", "OpImageWrite %im %vi12 %vi1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rg32sint, write>;)",
         "textureStore(x_20, vi12, vi1234);"},
        // WGSL does not support 3-component storage textures.
        // Source 4 component, dest 4 component
        {"%int 2D 0 0 0 2 Rgba32i", "OpImageWrite %im %vi12 %vi1234",
         "@group(2) @binding(1) var x_20 : "
         "texture_storage_2d<rgba32sint, write>;",
         "textureStore(x_20, vi12, vi1234);"}}));

TEST_F(SpvParserHandleTest, ImageWrite_TooFewSrcTexelComponents_1_vs_4) {
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %main "main"
     OpExecutionMode %main OriginUpperLeft
     OpName %f1 "f1"
     OpName %coords12 "coords12"
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
)" + CommonBasicTypes() +
                          R"(
     %im_ty = OpTypeImage %void 2D 0 0 0 2 Rgba32f
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty

     %20 = OpVariable %ptr_im_ty UniformConstant

     %main = OpFunction %void None %voidfn
     %entry = OpLabel

     %f1 = OpCopyObject %float %float_1

     %coords12 = OpCopyObject %v2float %the_vf12

     %im = OpLoad %im_ty %20
     OpImageWrite %im %coords12 %f1
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_THAT(p->error(), Eq("texel has too few components for storage texture: 1 provided "
                               "but 4 required, in: OpImageWrite %54 %3 %2"))
        << p->error();
}

INSTANTIATE_TEST_SUITE_P(
    // The texel operand signedness must match the channel type signedness.
    // SPIR-V validation checks that.
    // This suite is for the cases where they are integral and the same
    // signedness.
    ImageWrite_ConvertTexelOperand_SameSignedness,
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Sampled type is unsigned int, texel is unsigned int
        {"%uint 2D 0 0 0 2 Rgba32ui", "OpImageWrite %im %vi12 %vu1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32uint, write>;)",
         R"(textureStore(x_20, vi12, vu1234))"},
        // Sampled type is signed int, texel is signed int
        {"%int 2D 0 0 0 2 Rgba32i", "OpImageWrite %im %vi12 %vi1234",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32sint, write>;)",
         R"(textureStore(x_20, vi12, vi1234))"}}));

INSTANTIATE_TEST_SUITE_P(
    // Error out when OpImageWrite write texel differ in float vs. integral
    ImageWrite_ConvertTexelOperand_DifferentFloatishness_IsError,
    // Use the ImageDeclTest so we can check the error.
    SpvParserHandleTest_ImageDeclTest,
    ::testing::ValuesIn(std::vector<ImageDeclCase>{
        // Sampled type is float, texel is signed int
        {"%uint 2D 0 0 0 2 Rgba32f", "OpImageWrite %im %vi12 %vi1234",
         "invalid texel type for storage texture write: component must be "
         "float, signed integer, or unsigned integer to match the texture "
         "channel type: OpImageWrite",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32float, write>;)"},
        // Sampled type is float, texel is unsigned int
        {"%int 2D 0 0 0 2 Rgba32f", "OpImageWrite %im %vi12 %vu1234",
         "invalid texel type for storage texture write: component must be "
         "float, signed integer, or unsigned integer to match the texture "
         "channel type: OpImageWrite",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32float, write>;)"},
        // Sampled type is unsigned int, texel is float
        {"%uint 2D 0 0 0 2 Rgba32ui", "OpImageWrite %im %vi12 %vf1234",
         "invalid texel type for storage texture write: component must be "
         "float, signed integer, or unsigned integer to match the texture "
         "channel type: OpImageWrite",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32uint, write>;)"},
        // Sampled type is signed int, texel is float
        {"%int 2D 0 0 0 2 Rgba32i", "OpImageWrite %im %vi12 %vf1234",
         "invalid texel type for storage texture write: component must be "
         "float, signed integer, or unsigned integer to match the texture "
         "channel type: OpImageWrite",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32sint, write>;
  })"}}));

INSTANTIATE_TEST_SUITE_P(
    // Error out when OpImageWrite write texel signedness is different.
    ImageWrite_ConvertTexelOperand_DifferentSignedness_IsError,
    // Use the ImageDeclTest so we can check the error.
    SpvParserHandleTest_ImageDeclTest,
    ::testing::ValuesIn(std::vector<ImageDeclCase>{
        // Sampled type is unsigned int, texel is signed int
        {"%uint 2D 0 0 0 2 Rgba32ui", "OpImageWrite %im %vi12 %vi1234",
         "invalid texel type for storage texture write: component must be "
         "float, signed integer, or unsigned integer to match the texture "
         "channel type: OpImageWrite",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32uint, write>;)"},
        // Sampled type is signed int, texel is unsigned int
        {"%int 2D 0 0 0 2 Rgba32i", "OpImageWrite %im %vi12 %vu1234",
         "invalid texel type for storage texture write: component must be "
         "float, signed integer, or unsigned integer to match the texture "
         "channel type: OpImageWrite",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<rgba32sint, write>;
  })"}}));

INSTANTIATE_TEST_SUITE_P(
    // Show that zeros of the correct integer signedness are
    // created when expanding an integer vector.
    ImageWrite_ConvertTexelOperand_Signedness_AndWidening,
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Source unsigned, dest unsigned
        {"%uint 2D 0 0 0 2 R32ui", "OpImageWrite %im %vi12 %vu12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32uint, write>;)",
         R"(textureStore(x_20, vi12, vec4u(vu12, 0u, 0u)))"},
        // Source signed, dest signed
        {"%int 2D 0 0 0 2 R32i", "OpImageWrite %im %vi12 %vi12",
         R"(@group(2) @binding(1) var x_20 : texture_storage_2d<r32sint, write>;)",
         R"(textureStore(x_20, vi12, vec4i(vi12, 0i, 0i)))"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageFetch_OptionalParams,
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // OpImageFetch with no extra params, on sampled texture
        // Level of detail is injected for sampled texture
        {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12",
         R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
         R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},
        // OpImageFetch with explicit level, on sampled texture
        {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12 Lod %int_3",
         R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
         R"(let x_99 = textureLoad(x_20, vi12, 3i);)"},
        // OpImageFetch with no extra params, on depth texture
        // Level of detail is injected for depth texture
        {"%float 2D 1 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12",
         R"(@group(2) @binding(1) var x_20 : texture_depth_2d;)",
         R"(let x_99 = vec4f(textureLoad(x_20, vi12, 0i), 0.0f, 0.0f, 0.0f);)"},
        // OpImageFetch with extra params, on depth texture
        {"%float 2D 1 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12 Lod %int_3",
         R"(@group(2) @binding(1) var x_20 : texture_depth_2d;)",
         R"(let x_99 = vec4f(textureLoad(x_20, vi12, 3i), 0.0f, 0.0f, 0.0f);)"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageFetch_Depth,
    // In SPIR-V OpImageFetch always yields a vector of 4
    // elements, even for depth images.  But in WGSL,
    // textureLoad on a depth image yields f32.
    // crbug.com/tint/439
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // ImageFetch on depth image.
        {"%float 2D 1 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12 ",
         R"(@group(2) @binding(1) var x_20 : texture_depth_2d;)",
         R"(let x_99 = vec4f(textureLoad(x_20, vi12, 0i), 0.0f, 0.0f, 0.0f);)"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageFetch_DepthMultisampled,
    // In SPIR-V OpImageFetch always yields a vector of 4
    // elements, even for depth images.  But in WGSL,
    // textureLoad on a depth image yields f32.
    // crbug.com/tint/439
    SpvParserHandleTest_ImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // ImageFetch on multisampled depth image.
        {"%float 2D 1 0 1 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12 Sample %i1",
         R"(@group(2) @binding(1) var x_20 : texture_depth_multisampled_2d;)",
         R"(let x_99 = vec4f(textureLoad(x_20, vi12, i1), 0.0f, 0.0f, 0.0f);)"}}));

INSTANTIATE_TEST_SUITE_P(ImageFetch_Multisampled,
                         SpvParserHandleTest_ImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // SPIR-V requires a Sample image operand when operating on a
                             // multisampled image.

                             // ImageFetch arrayed
                             // Not in WebGPU

                             // ImageFetch non-arrayed
                             {"%float 2D 0 0 1 1 Unknown",
                              "%99 = OpImageFetch %v4float %im %vi12 Sample %i1",
                              R"(@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, i1);)"}}));

INSTANTIATE_TEST_SUITE_P(ImageFetch_Multisampled_ConvertSampleOperand,
                         SpvParserHandleTest_ImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             {"%float 2D 0 0 1 1 Unknown",
                              "%99 = OpImageFetch %v4float %im %vi12 Sample %u1",
                              R"(@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, i32(u1));)"}}));

INSTANTIATE_TEST_SUITE_P(ConvertResultSignedness,
                         SpvParserHandleTest_SampledImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // Valid SPIR-V only has:
                             //      float scalar sampled type vs. floating result
                             //      integral scalar sampled type vs. integral result
                             // Any of the sampling, reading, or fetching use the same codepath.

                             // We'll test with:
                             //     OpImageFetch
                             //     OpImageRead
                             //     OpImageSampleImplicitLod - representative of sampling

                             //
                             // OpImageRead
                             //

                             // OpImageFetch requires no conversion, float -> v4float
                             {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4float %im %vi12",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},
                             // OpImageFetch requires no conversion, uint -> v4uint
                             {"%uint 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4uint %im %vi12",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<u32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},
                             // OpImageFetch requires conversion, uint -> v4int
                             // is invalid SPIR-V:
                             // "Expected Image 'Sampled Type' to be the same as Result Type
                             // components"

                             // OpImageFetch requires no conversion, int -> v4int
                             {"%int 2D 0 0 0 1 Unknown", "%99 = OpImageFetch %v4int %im %vi12",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<i32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},
                             // OpImageFetch requires conversion, int -> v4uint
                             // is invalid SPIR-V:
                             // "Expected Image 'Sampled Type' to be the same as Result Type
                             // components"

                             //
                             // OpImageRead
                             //

                             // OpImageRead requires no conversion, float -> v4float
                             {"%float 2D 0 0 0 2 Rgba32f", "%99 = OpImageRead %v4float %im %vi12",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},
                             // OpImageRead requires no conversion, uint -> v4uint
                             {"%uint 2D 0 0 0 2 Rgba32ui", "%99 = OpImageRead %v4uint %im %vi12",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<u32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},

                             // OpImageRead requires conversion, uint -> v4int
                             // is invalid SPIR-V:
                             // "Expected Image 'Sampled Type' to be the same as Result Type
                             // components"

                             // OpImageRead requires no conversion, int -> v4int
                             {"%int 2D 0 0 0 2 Rgba32i", "%99 = OpImageRead %v4int %im %vi12",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<i32>;)",
                              R"(let x_99 = textureLoad(x_20, vi12, 0i);)"},

                             // OpImageRead requires conversion, int -> v4uint
                             // is invalid SPIR-V:
                             // "Expected Image 'Sampled Type' to be the same as Result Type
                             // components"

                             //
                             // Sampling operations, using OpImageSampleImplicitLod as an example.
                             // WGSL sampling operations only work on textures with a float sampled
                             // component.  So we can only test the float -> float (non-conversion)
                             // case.

                             // OpImageSampleImplicitLod requires no conversion, float -> v4float
                             {"%float 2D 0 0 0 1 Unknown",
                              "%99 = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
                              R"(@group(0) @binding(0) var x_10 : sampler;

@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                              R"(let x_99 = textureSample(x_20, x_10, vf12);)"}}));

INSTANTIATE_TEST_SUITE_P(ImageQuerySize_NonArrayed_SignedResult,
                         // ImageQuerySize requires storage image or multisampled
                         // For storage image, use another instruction to indicate whether it
                         // is readonly or writeonly.
                         SpvParserHandleTest_SampledImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // 1D storage image
                             {"%float 1D 0 0 0 2 Rgba32f",
                              "%99 = OpImageQuerySize %int %im \n"
                              "%98 = OpImageRead %v4float %im %i1\n",  // Implicitly mark as
                                                                       // NonWritable
                              R"(@group(2) @binding(1) var x_20 : texture_1d<f32>;)",
                              R"(let x_99 = i32(textureDimensions(x_20));)"},
                             // 2D storage image
                             {"%float 2D 0 0 0 2 Rgba32f",
                              "%99 = OpImageQuerySize %v2int %im \n"
                              "%98 = OpImageRead %v4float %im %vi12\n",  // Implicitly mark as
                                                                         // NonWritable
                              R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                              R"(let x_99 = vec2i(textureDimensions(x_20))"},
                             // 3D storage image
                             {"%float 3D 0 0 0 2 Rgba32f",
                              "%99 = OpImageQuerySize %v3int %im \n"
                              "%98 = OpImageRead %v4float %im %vi123\n",  // Implicitly mark as
                                                                          // NonWritable
                              R"(@group(2) @binding(1) var x_20 : texture_3d<f32>;)",
                              R"(let x_99 = vec3i(textureDimensions(x_20));)"},

                             // Multisampled
                             {"%float 2D 0 0 1 1 Unknown", "%99 = OpImageQuerySize %v2int %im \n",
                              R"(@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;)",
                              R"(let x_99 = vec2i(textureDimensions(x_20));)"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageQuerySize_Arrayed_SignedResult,
    // ImageQuerySize requires storage image or multisampled
    // For storage image, use another instruction to indicate whether it
    // is readonly or writeonly.
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // 1D array storage image doesn't exist.

        // 2D array storage image
        {"%float 2D 0 1 0 2 Rgba32f",
         "%99 = OpImageQuerySize %v3int %im \n"
         "%98 = OpImageRead %v4float %im %vi123\n",
         R"(@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
         R"(let x_99 = vec3i(vec3u(textureDimensions(x_20), textureNumLayers(x_20)));)"}
        // 3D array storage image doesn't exist.

        // Multisampled array
        // Not in WebGPU
    }));

INSTANTIATE_TEST_SUITE_P(ImageQuerySize_NonArrayed_UnsignedResult,
                         // ImageQuerySize requires storage image or multisampled
                         // For storage image, use another instruction to indicate whether it
                         // is readonly or writeonly.
                         SpvParserHandleTest_SampledImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // 1D storage image
                             {"%float 1D 0 0 0 2 Rgba32f",
                              "%99 = OpImageQuerySize %uint %im \n"
                              "%98 = OpImageRead %v4float %im %i1\n",  // Implicitly mark as
                                                                       // NonWritable
                              R"(@group(2) @binding(1) var x_20 : texture_1d<f32>;)",
                              R"(let x_99 = textureDimensions(x_20);)"},
                             // 2D storage image
                             {"%float 2D 0 0 0 2 Rgba32f",
                              "%99 = OpImageQuerySize %v2uint %im \n"
                              "%98 = OpImageRead %v4float %im %vi12\n",  // Implicitly mark as
                                                                         // NonWritable
                              R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                              R"(let x_99 = textureDimensions(x_20);)"},
                             // 3D storage image
                             {"%float 3D 0 0 0 2 Rgba32f",
                              "%99 = OpImageQuerySize %v3uint %im \n"
                              "%98 = OpImageRead %v4float %im %vi123\n",  // Implicitly mark as
                                                                          // NonWritable
                              R"(@group(2) @binding(1) var x_20 : texture_3d<f32>;)",
                              R"(let x_99 = textureDimensions(x_20);)"},

                             // Multisampled
                             {"%float 2D 0 0 1 1 Unknown", "%99 = OpImageQuerySize %v2uint %im \n",
                              R"(@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;)",
                              R"(let x_99 = textureDimensions(x_20);)"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageQuerySize_Arrayed_UnsignedResult,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // 1D array storage image doesn't exist.

        // 2D array storage image
        {"%float 2D 0 1 0 2 Rgba32f",
         "%99 = OpImageQuerySize %v3uint %im \n"
         "%98 = OpImageRead %v4float %im %vi123\n",
         R"(@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
         R"(let x_99 = vec3u(textureDimensions(x_20), textureNumLayers(x_20));)"}
        // 3D array storage image doesn't exist.

        // Multisampled array
        // Not in WebGPU
    }));

INSTANTIATE_TEST_SUITE_P(
    ImageQuerySizeLod_NonArrayed_SignedResult_SignedLevel,
    // From VUID-StandaloneSpirv-OpImageQuerySizeLod-04659:
    //  ImageQuerySizeLod requires Sampled=1
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // 1D
        {"%float 1D 0 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_1d<f32>;)",
         R"(let x_99 = i32(textureDimensions(x_20, i1)))"},

        // 2D
        {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %v2int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
         R"(let x_99 = vec2i(textureDimensions(x_20, i1));)"},

        // 3D
        {"%float 3D 0 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %v3int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_3d<f32>;)",
         R"(let x_99 = vec3i(textureDimensions(x_20, i1));)"},

        // Cube
        {"%float Cube 0 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %v2int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_cube<f32>;)",
         R"(let x_99 = vec2i(textureDimensions(x_20, i1).xy);)"},

        // Depth 2D
        {"%float 2D 1 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %v2int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_depth_2d;)",
         R"(let x_99 = vec2i(textureDimensions(x_20, i1));)"},

        // Depth Cube
        {"%float Cube 1 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %v2int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_depth_cube;)",
         R"(let x_99 = vec2i(textureDimensions(x_20, i1).xy);)"}}));

INSTANTIATE_TEST_SUITE_P(
    ImageQuerySizeLod_Arrayed_SignedResult_SignedLevel,
    // ImageQuerySize requires storage image or multisampled
    // For storage image, use another instruction to indicate whether it
    // is readonly or writeonly.
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{

        // There is no 1D array

        // 2D array
        {"%float 2D 0 1 0 1 Unknown", "%99 = OpImageQuerySizeLod %v3int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
         R"(let x_99 = vec3i(vec3u(textureDimensions(x_20, i1), textureNumLayers(x_20)));)"},

        // There is no 3D array

        // Cube array
        //
        // Currently textureDimension on cube returns vec3 but maybe should
        // return vec2
        // https://github.com/gpuweb/gpuweb/issues/1345
        {"%float Cube 0 1 0 1 Unknown", "%99 = OpImageQuerySizeLod %v3int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_cube_array<f32>;)",
         R"(let x_99 = vec3i(vec3u(textureDimensions(x_20, i1).xy, textureNumLayers(x_20)));)"},

        // Depth 2D array
        {"%float 2D 1 1 0 1 Unknown", "%99 = OpImageQuerySizeLod %v3int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
         R"(let x_99 = vec3i(vec3u(textureDimensions(x_20, i1), textureNumLayers(x_20)));)"},

        // Depth Cube Array
        //
        // Currently textureDimension on cube returns vec3 but maybe should
        // return vec2
        // https://github.com/gpuweb/gpuweb/issues/1345
        {"%float Cube 1 1 0 1 Unknown", "%99 = OpImageQuerySizeLod %v3int %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_depth_cube_array;)",
         R"(let x_99 = vec3i(vec3u(textureDimensions(x_20, i1).xy, textureNumLayers(x_20)));)"}}));

INSTANTIATE_TEST_SUITE_P(
    // textureDimensions accepts both signed and unsigned the level-of-detail values.
    ImageQuerySizeLod_NonArrayed_SignedResult_UnsignedLevel,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{

        {"%float 1D 0 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %int %im %u1\n",
         R"(@group(2) @binding(1) var x_20 : texture_1d<f32>;)",
         R"(let x_99 = i32(textureDimensions(x_20, u1));)"}}));

INSTANTIATE_TEST_SUITE_P(
    // When SPIR-V wants the result type to be unsigned, we have to
    // insert a value constructor or bitcast for WGSL to do the type
    // coercion. But the algorithm already does that as a matter
    // of course.
    ImageQuerySizeLod_NonArrayed_UnsignedResult_SignedLevel,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{

        {"%float 1D 0 0 0 1 Unknown", "%99 = OpImageQuerySizeLod %uint %im %i1\n",
         R"(@group(2) @binding(1) var x_20 : texture_1d<f32>;)",
         R"(let x_99 = textureDimensions(x_20, i1);)"}}));

INSTANTIATE_TEST_SUITE_P(ImageQueryLevels_SignedResult,
                         SpvParserHandleTest_SampledImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // In Vulkan:
                             //      Dim must be 1D, 2D, 3D, Cube
                             // WGSL allows 2d, 2d_array, 3d, cube, cube_array
                             // depth_2d, depth_2d_array, depth_cube, depth_cube_array

                             // 2D
                             {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // 2D array
                             {"%float 2D 0 1 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_2d_array<f32>;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // 3D
                             {"%float 3D 0 0 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_3d<f32>;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // Cube
                             {"%float Cube 0 0 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_cube<f32>;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // Cube array
                             {"%float Cube 0 1 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_cube_array<f32>;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // depth 2d
                             {"%float 2D 1 0 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_depth_2d;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // depth 2d array
                             {"%float 2D 1 1 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_depth_2d_array;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // depth cube
                             {"%float Cube 1 0 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_depth_cube;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"},

                             // depth cube array
                             {"%float Cube 1 1 0 1 Unknown", "%99 = OpImageQueryLevels %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_depth_cube_array;)",
                              R"(let x_99 = i32(textureNumLevels(x_20));)"}}));

INSTANTIATE_TEST_SUITE_P(
    // Spot check that a value conversion is inserted when SPIR-V asks for an unsigned int result.
    ImageQueryLevels_UnsignedResult,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        {"%float 2D 0 0 0 1 Unknown", "%99 = OpImageQueryLevels %uint %im\n",
         R"(@group(2) @binding(1) var x_20 : texture_2d<f32>;)",
         R"(let x_99 = textureNumLevels(x_20);)"}}));

INSTANTIATE_TEST_SUITE_P(ImageQuerySamples_SignedResult,
                         SpvParserHandleTest_SampledImageAccessTest,
                         ::testing::ValuesIn(std::vector<ImageAccessCase>{
                             // Multsample 2D
                             {"%float 2D 0 0 1 1 Unknown", "%99 = OpImageQuerySamples %int %im\n",
                              R"(@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;)",
                              R"(let x_99 = i32(textureNumSamples(x_20));)"}

                             // Multisample 2D array
                             // Not in WebGPU
                         }));

INSTANTIATE_TEST_SUITE_P(
    // Translation must inject a type coersion from unsigned to signed.
    ImageQuerySamples_UnsignedResult,
    SpvParserHandleTest_SampledImageAccessTest,
    ::testing::ValuesIn(std::vector<ImageAccessCase>{
        // Multisample 2D
        {"%float 2D 0 0 1 1 Unknown", "%99 = OpImageQuerySamples %uint %im\n",
         R"(@group(2) @binding(1) var x_20 : texture_multisampled_2d<f32>;)",
         R"(let x_99 = textureNumSamples(x_20);)"}

        // Multisample 2D array
        // Not in WebGPU
    }));

struct ImageCoordsCase {
    // SPIR-V image type, excluding result ID and opcode
    std::string spirv_image_type_details;
    std::string spirv_image_access;
    std::string expected_error;
    std::vector<std::string> expected_expressions;
};

inline std::ostream& operator<<(std::ostream& out, const ImageCoordsCase& c) {
    out << "ImageCoordsCase(" << c.spirv_image_type_details << "\n"
        << c.spirv_image_access << "\n"
        << "expected_error(" << c.expected_error << ")\n";

    for (auto e : c.expected_expressions) {
        out << e << ",";
    }
    out << ")\n";
    return out;
}

using SpvParserHandleTest_ImageCoordsTest =
    SpirvASTParserTestBase<::testing::TestWithParam<ImageCoordsCase>>;

TEST_P(SpvParserHandleTest_ImageCoordsTest, MakeCoordinateOperandsForImageAccess) {
    // Only declare the sampled image type, and the associated variable
    // if the requested image type is a sampled image type and not multisampled.
    const bool is_sampled_image_type =
        GetParam().spirv_image_type_details.find("0 1 Unknown") != std::string::npos;
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft
     OpName %float_var "float_var"
     OpName %ptr_float "ptr_float"
     OpName %i1 "i1"
     OpName %vi12 "vi12"
     OpName %vi123 "vi123"
     OpName %vi1234 "vi1234"
     OpName %u1 "u1"
     OpName %vu12 "vu12"
     OpName %vu123 "vu123"
     OpName %vu1234 "vu1234"
     OpName %f1 "f1"
     OpName %vf12 "vf12"
     OpName %vf123 "vf123"
     OpName %vf1234 "vf1234"
     OpDecorate %10 DescriptorSet 0
     OpDecorate %10 Binding 0
     OpDecorate %20 DescriptorSet 2
     OpDecorate %20 Binding 1
     OpDecorate %30 DescriptorSet 0
     OpDecorate %30 Binding 1
)" + CommonBasicTypes() +
                          R"(
     %sampler = OpTypeSampler
     %ptr_sampler = OpTypePointer UniformConstant %sampler
     %im_ty = OpTypeImage )" +
                          GetParam().spirv_image_type_details + R"(
     %ptr_im_ty = OpTypePointer UniformConstant %im_ty
)" + (is_sampled_image_type ? " %si_ty = OpTypeSampledImage %im_ty " : "") +
                          R"(

     %ptr_float = OpTypePointer Function %float

     %10 = OpVariable %ptr_sampler UniformConstant
     %20 = OpVariable %ptr_im_ty UniformConstant
     %30 = OpVariable %ptr_sampler UniformConstant ; comparison sampler, when needed

     %100 = OpFunction %void None %voidfn
     %entry = OpLabel

     %float_var = OpVariable %ptr_float Function

     %i1 = OpCopyObject %int %int_1
     %vi12 = OpCopyObject %v2int %the_vi12
     %vi123 = OpCopyObject %v3int %the_vi123
     %vi1234 = OpCopyObject %v4int %the_vi1234

     %u1 = OpCopyObject %uint %uint_1
     %vu12 = OpCopyObject %v2uint %the_vu12
     %vu123 = OpCopyObject %v3uint %the_vu123
     %vu1234 = OpCopyObject %v4uint %the_vu1234

     %f1 = OpCopyObject %float %float_1
     %vf12 = OpCopyObject %v2float %the_vf12
     %vf123 = OpCopyObject %v3float %the_vf123
     %vf1234 = OpCopyObject %v4float %the_vf1234

     %sam = OpLoad %sampler %10
     %im = OpLoad %im_ty %20

)" + (is_sampled_image_type ? " %sampled_image = OpSampledImage %si_ty %im %sam " : "") +
                          GetParam().spirv_image_access +
                          R"(
     ; Use an anchor for the cases when the image access doesn't have a result ID.
     %1000 = OpCopyObject %uint %uint_0

     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    if (!p->BuildAndParseInternalModule()) {
        EXPECT_THAT(p->error(), StartsWith(GetParam().expected_error)) << assembly;
    } else {
        EXPECT_TRUE(p->error().empty()) << p->error();
        auto fe = p->function_emitter(100);
        // We actually have to generate the module to cache expressions for the
        // result IDs, particularly the OpCopyObject
        fe.Emit();

        const spvtools::opt::Instruction* anchor = p->GetInstructionForTest(1000);
        ASSERT_NE(anchor, nullptr);
        const spvtools::opt::Instruction& image_access = *(anchor->PreviousNode());

        auto result = fe.MakeCoordinateOperandsForImageAccess(image_access);
        if (GetParam().expected_error.empty()) {
            EXPECT_TRUE(fe.success()) << p->error();
            EXPECT_TRUE(p->error().empty());
            std::vector<std::string> result_strings;
            Program program = p->program();
            for (auto* expr : result) {
                ASSERT_NE(expr, nullptr);
                result_strings.push_back(test::ToString(program, expr));
            }
            EXPECT_THAT(result_strings, ::testing::ContainerEq(GetParam().expected_expressions));
        } else {
            EXPECT_FALSE(fe.success());
            EXPECT_THAT(p->error(), Eq(GetParam().expected_error)) << assembly;
            EXPECT_TRUE(result.IsEmpty());
        }
    }

    const bool is_sample_level =
        GetParam().spirv_image_access.find("ImageSampleExplicitLod") != std::string::npos;
    const bool is_comparison_sample_level =
        GetParam().spirv_image_access.find("ImageSampleDrefExplicitLod") != std::string::npos;
    const bool is_1d = GetParam().spirv_image_type_details.find("1D") != std::string::npos;
    if (is_sample_level && is_1d) {
        p->SkipDumpingPending("crbug.com/tint/789");
    }
    if (is_comparison_sample_level) {
        p->SkipDumpingPending("crbug.com/tint/425");
    }
}

INSTANTIATE_TEST_SUITE_P(Good_1D,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %f1",
                              "",
                              {"f1"}},
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf12",  // one excess arg
                              "",
                              {"vf12.x"}},
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",  // two excess args
                              "",
                              {"vf123.x"}},
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // three excess args
                              "",
                              {"vf1234.x"}}}));

INSTANTIATE_TEST_SUITE_P(Good_1DArray,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 1D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf12",
                              "",
                              {"vf12.x", "i32(round(vf12.y))"}},
                             {"%float 1D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",  // one excess arg
                              "",
                              {"vf123.x", "i32(round(vf123.y))"}},
                             {"%float 1D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // two excess args
                              "",
                              {"vf1234.x", "i32(round(vf1234.y))"}}}));

INSTANTIATE_TEST_SUITE_P(Good_2D,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf12",
                              "",
                              {"vf12"}},
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",  // one excess arg
                              "",
                              {"vf123.xy"}},
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // two excess args
                              "",
                              {"vf1234.xy"}}}));

INSTANTIATE_TEST_SUITE_P(Good_2DArray,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf123",
                              "",
                              {"vf123.xy", "i32(round(vf123.z))"}},
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float "
                              "%sampled_image %vf1234",  // one excess arg
                              "",
                              {"vf1234.xy", "i32(round(vf1234.z))"}}}));

INSTANTIATE_TEST_SUITE_P(Good_3D,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 3D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf123",
                              "",
                              {"vf123"}},
                             {"%float 3D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf1234",  // one excess
                                                         // arg
                              "",
                              {"vf1234.xyz"}}}));

INSTANTIATE_TEST_SUITE_P(Good_Cube,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float Cube 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf123",
                              "",
                              {"vf123"}},
                             {"%float Cube 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf1234",  // one excess
                                                         // arg
                              "",
                              {"vf1234.xyz"}}}));

INSTANTIATE_TEST_SUITE_P(Good_CubeArray,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float Cube 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              "%v4float "
                              "%sampled_image %vf1234",
                              "",
                              {"vf1234.xyz", "i32(round(vf1234.w))"}}}));

INSTANTIATE_TEST_SUITE_P(
    PreserveFloatCoords_NonArrayed,
    // In SPIR-V, sampling and dref sampling operations use floating point
    // coordinates.  Prove that we preserve floating point-ness.
    // Test across all such instructions.
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Scalar cases
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %f1",
         "",
         {"f1"}},
        {"%float 1D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float %sampled_image %f1 Lod "
         "%f1",
         "",
         {"f1"}},
        // WGSL does not support depth textures with 1D coordinates
        // Vector cases
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf12",
         "",
         {"vf12"}},
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageSampleExplicitLod %v4float %sampled_image %vf12 Lod "
         "%f1",
         "",
         {"vf12"}},
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image %vf12 "
         "%depth",
         "",
         {"vf12"}},
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf12 "
         "%depth Lod %float_0",
         "",
         {"vf12"}},
    }));

INSTANTIATE_TEST_SUITE_P(PreserveFloatCoords_Arrayed,
                         // In SPIR-V, sampling and dref sampling operations use floating point
                         // coordinates.  Prove that we preserve floating point-ness of the
                         // coordinate part, but convert the array index to signed integer. Test
                         // across all such instructions.
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf123",
                              "",
                              {"vf123.xy", "i32(round(vf123.z))"}},

                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageSampleExplicitLod %v4float %sampled_image %vf123 "
                              "Lod %f1",
                              "",
                              {"vf123.xy", "i32(round(vf123.z))"}},
                             {"%float 2D 1 1 0 1 Unknown",
                              "%result = OpImageSampleDrefImplicitLod %float %sampled_image "
                              "%vf123 %depth",
                              "",
                              {"vf123.xy", "i32(round(vf123.z))"}},
                             {"%float 2D 1 1 0 1 Unknown",
                              "%result = OpImageSampleDrefExplicitLod %float %sampled_image "
                              "%vf123 %depth Lod %float_0",
                              "",
                              {"vf123.xy", "i32(round(vf123.z))"}}}));

INSTANTIATE_TEST_SUITE_P(
    PreserveIntCoords_NonArrayed,
    // In SPIR-V, image read, fetch, and write use integer coordinates.
    // Prove that we preserve signed integer coordinates.
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Scalar cases
        {"%float 1D 0 0 0 1 Unknown", "%result = OpImageFetch %v4float %im %i1", "", {"i1"}},
        {"%float 1D 0 0 0 2 R32f", "%result = OpImageRead %v4float %im %i1", "", {"i1"}},
        {"%float 1D 0 0 0 2 R32f", "OpImageWrite %im %i1 %vf1234", "", {"i1"}},
        // Vector cases
        {"%float 2D 0 0 0 1 Unknown", "%result = OpImageFetch %v4float %im %vi12", "", {"vi12"}},
        {"%float 2D 0 0 0 2 R32f", "%result = OpImageRead %v4float %im %vi12", "", {"vi12"}},
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vi12 %vf1234", "", {"vi12"}}}));

INSTANTIATE_TEST_SUITE_P(PreserveIntCoords_Arrayed,
                         // In SPIR-V, image read, fetch, and write use integer coordinates.
                         // Prove that we preserve signed integer coordinates.
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageFetch %v4float %im %vi123",
                              "",
                              {"vi123.xy", "vi123.z"}},
                             {"%float 2D 0 1 0 2 R32f",
                              "%result = OpImageRead %v4float %im %vi123",
                              "",
                              {"vi123.xy", "vi123.z"}},
                             {"%float 2D 0 1 0 2 R32f",
                              "OpImageWrite %im %vi123 %vf1234",
                              "",
                              {"vi123.xy", "vi123.z"}}}));

INSTANTIATE_TEST_SUITE_P(
    ConvertUintCoords_NonArrayed,
    // In SPIR-V, image read, fetch, and write use integer coordinates.
    // Prove that we convert unsigned integer coordinates to signed.
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Scalar cases
        {"%float 1D 0 0 0 1 Unknown", "%result = OpImageFetch %v4float %im %u1", "", {"i32(u1)"}},
        {"%float 1D 0 0 0 2 R32f", "%result = OpImageRead %v4float %im %u1", "", {"i32(u1)"}},
        {"%float 1D 0 0 0 2 R32f", "OpImageWrite %im %u1 %vf1234", "", {"i32(u1)"}},
        // Vector cases
        {"%float 2D 0 0 0 1 Unknown",
         "%result = OpImageFetch %v4float %im %vu12",
         "",
         {"vec2i(vu12)"}},
        {"%float 2D 0 0 0 2 R32f", "%result = OpImageRead %v4float %im %vu12", "", {"vec2i(vu12)"}},
        {"%float 2D 0 0 0 2 R32f", "OpImageWrite %im %vu12 %vf1234", "", {"vec2i(vu12)"}}}));

INSTANTIATE_TEST_SUITE_P(ConvertUintCoords_Arrayed,
                         // In SPIR-V, image read, fetch, and write use integer coordinates.
                         // Prove that we convert unsigned integer coordinates to signed.
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 1 0 1 Unknown",
                              "%result = OpImageFetch %v4float %im %vu123",
                              "",
                              {"vec2i(vu123.xy)", "i32(vu123.z)"}},
                             {"%float 2D 0 1 0 2 R32f",
                              "%result = OpImageRead %v4float %im %vu123",
                              "",
                              {"vec2i(vu123.xy)", "i32(vu123.z)"}},
                             {"%float 2D 0 1 0 2 R32f",
                              "OpImageWrite %im %vu123 %vf1234",
                              "",
                              {"vec2i(vu123.xy)", "i32(vu123.z)"}}}));

INSTANTIATE_TEST_SUITE_P(
    BadInstructions,
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        {"%float 1D 0 0 0 1 Unknown", "OpNop", "not an image access instruction: OpNop", {}},
        {"%float 1D 0 0 0 1 Unknown",
         "%50 = OpCopyObject %float %float_1",
         "internal error: couldn't find image for "
         "%50 = OpCopyObject %18 %45",
         {}},
        {"%float 1D 0 0 0 1 Unknown",
         "OpStore %float_var %float_1",
         "invalid type for image or sampler "
         "variable or function parameter: %1 = OpVariable %2 Function",
         {}},
        // An example with a missing coordinate
        // won't assemble, so we skip it.
    }));

INSTANTIATE_TEST_SUITE_P(Bad_Coordinate,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod "
                              // bad type for coordinate: not a number
                              // %10 is the sampler variable
                              "%v4float %sampled_image %10",
                              "bad or unsupported coordinate type for image access: %73 = "
                              "OpImageSampleImplicitLod %42 %72 %10",
                              {}},
                             {"%float 2D 0 0 0 1 Unknown",  // 2D
                              "%result = OpImageSampleImplicitLod "
                              // 1 component, but need 2
                              "%v4float %sampled_image %f1",
                              "image access required 2 coordinate components, but only 1 provided, "
                              "in: %73 = OpImageSampleImplicitLod %42 %72 %12",
                              {}},
                             {"%float 2D 0 1 0 1 Unknown",  // 2DArray
                              "%result = OpImageSampleImplicitLod "
                              // 2 component, but need 3
                              "%v4float %sampled_image %vf12",
                              "image access required 3 coordinate components, but only 2 provided, "
                              "in: %73 = OpImageSampleImplicitLod %42 %72 %13",
                              {}},
                             {"%float 3D 0 0 0 1 Unknown",  // 3D
                              "%result = OpImageSampleImplicitLod "
                              // 2 components, but need 3
                              "%v4float %sampled_image %vf12",
                              "image access required 3 coordinate components, but only 2 provided, "
                              "in: %73 = OpImageSampleImplicitLod %42 %72 %13",
                              {}},
                         }));

INSTANTIATE_TEST_SUITE_P(SampleNonFloatTexture_IsError,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             // ImageSampleImplicitLod
                             {"%uint 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4uint %sampled_image %vf12",
                              "sampled image must have float component type",
                              {}},
                             {"%int 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4int %sampled_image %vf12",
                              "sampled image must have float component type",
                              {}},
                             // ImageSampleExplicitLod
                             {"%uint 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleExplicitLod %v4uint %sampled_image %vf12 "
                              "Lod %f1",
                              "sampled image must have float component type",
                              {}},
                             {"%int 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleExplicitLod %v4int %sampled_image %vf12 "
                              "Lod %f1",
                              "sampled image must have float component type",
                              {}},
                             // ImageSampleDrefImplicitLod
                             {"%uint 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleDrefImplicitLod %uint %sampled_image %vf12 "
                              "%f1",
                              "sampled image must have float component type",
                              {}},
                             {"%int 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleDrefImplicitLod %int %sampled_image %vf12 "
                              "%f1",
                              "sampled image must have float component type",
                              {}},
                             // ImageSampleDrefExplicitLod
                             {"%uint 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleDrefExplicitLod %uint %sampled_image %vf12 "
                              "%f1 Lod %float_0",
                              "sampled image must have float component type",
                              {}},
                             {"%int 2D 0 0 0 1 Unknown",
                              "%result = OpImageSampleDrefExplicitLod %int %sampled_image %vf12 "
                              "%f1 Lod %float_0",
                              "sampled image must have float component type",
                              {}}}));

INSTANTIATE_TEST_SUITE_P(ConstOffset_BadInstruction_Errors,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             // ImageFetch
                             {"%uint 2D 0 0 0 1 Unknown",
                              "%result = OpImageFetch %v4uint %sampled_image %vf12 ConstOffset "
                              "%the_vu12",
                              "ConstOffset is only permitted for sampling, gather, or "
                              "depth-reference gather operations: ",
                              {}},
                             // ImageRead
                             {"%uint 2D 0 0 0 2 Rgba32ui",
                              "%result = OpImageRead %v4uint %im %vu12 ConstOffset %the_vu12",
                              "ConstOffset is only permitted for sampling, gather, or "
                              "depth-reference gather operations: ",
                              {}},
                             // ImageWrite
                             {"%uint 2D 0 0 0 2 Rgba32ui",
                              "OpImageWrite %im %vu12 %vu1234 ConstOffset %the_vu12",
                              "ConstOffset is only permitted for sampling, gather, or "
                              "depth-reference gather operations: ",
                              {}}}));

INSTANTIATE_TEST_SUITE_P(ConstOffset_BadDim_Errors,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             // 1D
                             {"%uint 1D 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
                              "ConstOffset %the_vu12",
                              "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
                              {}},
                             // Cube
                             {"%uint Cube 0 0 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
                              "ConstOffset %the_vu12",
                              "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
                              {}},
                             // Cube Array
                             {"%uint Cube 0 1 0 1 Unknown",
                              "%result = OpImageSampleImplicitLod %v4float %sampled_image %vf1234 "
                              "ConstOffset %the_vu12",
                              "ConstOffset is only permitted for 2D, 2D Arrayed, and 3D textures: ",
                              {}}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDref_Bias_IsError,
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Implicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image %vf1234 "
         "%depth Bias %float_null",
         "WGSL does not support depth-reference sampling with level-of-detail "
         "bias: ",
         {}},
        // Explicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod|Bias %float_null %float_null",
         "WGSL does not support depth-reference sampling with level-of-detail "
         "bias: ",
         {}}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDref_Grad_IsError,
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // Implicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefImplicitLod %float %sampled_image %vf1234 "
         "%depth Grad %float_1 %float_2",
         "WGSL does not support depth-reference sampling with explicit "
         "gradient: ",
         {}},
        // Explicit Lod
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod|Grad %float_null %float_1  %float_2",
         "WGSL does not support depth-reference sampling with explicit "
         "gradient: ",
         {}}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleDrefExplicitLod_CheckForLod0,
    // Metal requires comparison sampling with explicit Level-of-detail to use
    // Lod 0.  The SPIR-V reader requires the operand to be parsed as a constant
    // 0 value. SPIR-V validation requires the Lod parameter to be a floating
    // point value for non-fetch operations. So only test float values.
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // float 0.0 works
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod %float_0",
         "",
         {"vf1234.xy"}},
        // float null works
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod %float_0",
         "",
         {"vf1234.xy"}},
        // float 1.0 fails.
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleDrefExplicitLod %float %sampled_image %vf1234 "
         "%depth Lod %float_1",
         "WGSL comparison sampling without derivatives requires "
         "level-of-detail "
         "0.0",
         {}}}));

INSTANTIATE_TEST_SUITE_P(
    ImageSampleProjDrefExplicitLod_CheckForLod0,
    // This is like the previous test, but for Projection sampling.
    //
    // Metal requires comparison sampling with explicit Level-of-detail to use
    // Lod 0.  The SPIR-V reader requires the operand to be parsed as a constant
    // 0 value. SPIR-V validation requires the Lod parameter to be a floating
    // point value for non-fetch operations. So only test float values.
    SpvParserHandleTest_ImageCoordsTest,
    ::testing::ValuesIn(std::vector<ImageCoordsCase>{
        // float 0.0 works
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleProjDrefExplicitLod %float %sampled_image "
         "%vf1234 %depth Lod %float_0",
         "",
         {"(vf1234.xy / vf1234.z)"}},
        // float null works
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleProjDrefExplicitLod %float %sampled_image "
         "%vf1234 %depth Lod %float_0",
         "",
         {"(vf1234.xy / vf1234.z)"}},
        // float 1.0 fails.
        {"%float 2D 1 0 0 1 Unknown",
         "%result = OpImageSampleProjDrefExplicitLod %float %sampled_image "
         "%vf1234 %depth Lod %float_1",
         "WGSL comparison sampling without derivatives requires "
         "level-of-detail "
         "0.0",
         {}}}));

TEST_F(SpvParserHandleTest, CombinedImageSampler_IsError) {
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpDecorate %var DescriptorSet 0
     OpDecorate %var Binding 0
  %float = OpTypeFloat 32
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
 %ptr_si = OpTypePointer UniformConstant %si
    %var = OpVariable %ptr_si UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
           OpReturn
           OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule()) << assembly;
    EXPECT_THAT(p->error(), HasSubstr("WGSL does not support combined image-samplers: "));
}

INSTANTIATE_TEST_SUITE_P(ImageQueryLod_IsError,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageQueryLod %v2int %sampled_image %vf12",
                              "WGSL does not support querying the level of detail of an image: ",
                              {}}}));

INSTANTIATE_TEST_SUITE_P(ImageGather_Bias_IsError,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageGather %v4float %sampled_image %vf12 %int_1 "
                              "Bias %float_null",
                              "WGSL does not support image gather with level-of-detail bias: ",
                              {}}}));

INSTANTIATE_TEST_SUITE_P(ImageDrefGather_Bias_IsError,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 1 0 0 1 Unknown",
                              "%result = OpImageDrefGather %v4float %sampled_image %vf12 %depth "
                              "Bias %float_null",
                              "WGSL does not support image gather with level-of-detail bias: ",
                              {}}}));

// Note: Vulkan SPIR-V ImageGather and ImageDrefGather do not allow explicit
// Lod. The SPIR-V validator should reject those cases already.

INSTANTIATE_TEST_SUITE_P(ImageGather_Grad_IsError,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 0 0 0 1 Unknown",
                              "%result = OpImageGather %v4float %sampled_image %vf12 %int_1 "
                              "Grad %vf12 %vf12",
                              "WGSL does not support image gather with explicit gradient: ",
                              {}}}));

INSTANTIATE_TEST_SUITE_P(ImageDrefGather_Grad_IsError,
                         SpvParserHandleTest_ImageCoordsTest,
                         ::testing::ValuesIn(std::vector<ImageCoordsCase>{
                             {"%float 2D 1 0 0 1 Unknown",
                              "%result = OpImageDrefGather %v4float %sampled_image %vf12 %depth "
                              "Grad %vf12 %vf12",
                              "WGSL does not support image gather with explicit gradient: ",
                              {}}}));

TEST_F(SpvParserHandleTest, NeverGenerateConstDeclForHandle_UseVariableDirectly) {
    // An ad-hoc test to prove we never had the issue
    // feared in crbug.com/tint/265.
    // Never create a const-declaration for a pointer to
    // a texture or sampler. Code generation always
    // traces back to the memory object declaration.
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpName %var "var"
     OpDecorate %var_im DescriptorSet 0
     OpDecorate %var_im Binding 0
     OpDecorate %var_s DescriptorSet 0
     OpDecorate %var_s Binding 1
  %float = OpTypeFloat 32
  %v4float = OpTypeVector %float 4
  %v2float = OpTypeVector %float 2
  %v2_0 = OpConstantNull %v2float
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
      %s = OpTypeSampler
 %ptr_im = OpTypePointer UniformConstant %im
  %ptr_s = OpTypePointer UniformConstant %s
 %var_im = OpVariable %ptr_im UniformConstant
  %var_s = OpVariable %ptr_s UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
 %ptr_v4 = OpTypePointer Function %v4float

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
    %var = OpVariable %ptr_v4 Function

; Try to induce generating a const-declaration of a pointer to
; a sampler or texture.

 %var_im_copy = OpCopyObject %ptr_im %var_im
  %var_s_copy = OpCopyObject %ptr_s %var_s

         %im0 = OpLoad %im %var_im_copy
          %s0 = OpLoad %s %var_s_copy
         %si0 = OpSampledImage %si %im0 %s0
          %t0 = OpImageSampleImplicitLod %v4float %si0 %v2_0


         %im1 = OpLoad %im %var_im_copy
          %s1 = OpLoad %s %var_s_copy
         %si1 = OpSampledImage %si %im1 %s1
          %t1 = OpImageSampleImplicitLod %v4float %si1 %v2_0

         %sum = OpFAdd %v4float %t0 %t1
           OpStore %var %sum

           OpReturn
           OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var var_1 : vec4f;
let x_22 = textureSample(x_2, x_3, vec2f());
let x_26 = textureSample(x_2, x_3, vec2f());
var_1 = (x_22 + x_26);
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, SamplerLoadedInEnclosingConstruct_DontGenerateVar) {
    // crbug.com/tint/1839
    // When a sampler is loaded in an enclosing structued construct, don't
    // generate a variable for it. The ordinary tracing logic will find the
    // originating variable anyway.
    const auto assembly = Preamble() + R"(
     OpEntryPoint Fragment %100 "main"
     OpExecutionMode %100 OriginUpperLeft

     OpName %var_im "var_im"
     OpName %var_s "var_s"
     OpDecorate %var_im DescriptorSet 0
     OpDecorate %var_im Binding 0
     OpDecorate %var_s DescriptorSet 0
     OpDecorate %var_s Binding 1
  %float = OpTypeFloat 32
  %v4float = OpTypeVector %float 4
  %v2float = OpTypeVector %float 2
  %v2_0 = OpConstantNull %v2float
     %im = OpTypeImage %float 2D 0 0 0 1 Unknown
     %si = OpTypeSampledImage %im
      %s = OpTypeSampler
 %ptr_im = OpTypePointer UniformConstant %im
  %ptr_s = OpTypePointer UniformConstant %s
 %var_im = OpVariable %ptr_im UniformConstant
  %var_s = OpVariable %ptr_s UniformConstant
   %void = OpTypeVoid
 %voidfn = OpTypeFunction %void
 %ptr_v4 = OpTypePointer Function %v4float
   %bool = OpTypeBool
   %true = OpConstantTrue %bool
    %int = OpTypeInt 32 1
  %int_0 = OpConstant %int 0

    %100 = OpFunction %void None %voidfn
  %entry = OpLabel
      %1 = OpLoad %im %var_im
      %2 = OpLoad %s %var_s
           OpSelectionMerge %90 None
           OpSwitch %int_0 %20

     %20 = OpLabel
           OpSelectionMerge %80 None
           OpBranchConditional %true %30 %80

     %30 = OpLabel
    %si0 = OpSampledImage %si %1 %2
     %t0 = OpImageSampleImplicitLod %v4float %si0 %v2_0
           OpBranch %80

     %80 = OpLabel
           OpBranch %90

     %90 = OpLabel
           OpReturn
           OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(switch(0i) {
  default: {
    if (true) {
      let x_24 = textureSample(var_im, var_s, vec2f());
    }
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, ImageCoordinateCanBeHoistedConstant) {
    // Demonstrates fix for crbug.com/tint/1646
    // The problem is the coordinate for an image operation
    // can be a combinatorial value that has been hoisted out
    // to a 'var' declaration.
    //
    // In this test (and the original case form the bug), the
    // definition for the value is in an outer construct, and
    // the image operation using it is in a doubly nested
    // construct.
    //
    // The coordinate handling has to unwrap the ref type it
    // made for the 'var' declaration.
    const auto assembly = Preamble() + R"(

OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpDecorate %10 DescriptorSet 0
OpDecorate %10 Binding 0
OpDecorate %20 DescriptorSet 2
OpDecorate %20 Binding 1

%void = OpTypeVoid
%voidfn = OpTypeFunction %void

%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32

%v4float = OpTypeVector %float 4

%float_null = OpConstantNull %float

%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
%im_ty = OpTypeImage %float 1D 0 0 0 1 Unknown
%ptr_im_ty = OpTypePointer UniformConstant %im_ty
%si_ty = OpTypeSampledImage %im_ty

%10 = OpVariable %ptr_sampler UniformConstant
%20 = OpVariable %ptr_im_ty UniformConstant

%100 = OpFunction %void None %voidfn
%entry = OpLabel
%900 = OpCopyObject %float %float_null        ; definition here
OpSelectionMerge %99 None
OpBranchConditional %true %40 %99

  %40 = OpLabel
  OpSelectionMerge %80 None
  OpBranchConditional %true %50 %80

    %50 = OpLabel
    %sam = OpLoad %sampler %10
    %im = OpLoad %im_ty %20
    %sampled_image = OpSampledImage %si_ty %im %sam
    %result = OpImageSampleImplicitLod %v4float %sampled_image %900 ; usage here
    OpBranch %80

  %80 = OpLabel
  OpBranch %99

%99 = OpLabel
OpReturn
OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_900 : f32;
x_900 = 0.0f;
if (true) {
  if (true) {
    let x_18 = textureSample(x_20, x_10, x_900);
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, ImageCoordinateCanBeHoistedVector) {
    // Demonstrates fix for crbug.com/tint/1712
    // The problem is the coordinate for an image operation
    // can be a combinatorial value that has been hoisted out
    // to a 'var' declaration.
    //
    // In this test (and the original case form the bug), the
    // definition for the value is in an outer construct, and
    // the image operation using it is in a doubly nested
    // construct.
    //
    // The coordinate handling has to unwrap the ref type it
    // made for the 'var' declaration.
    const auto assembly = Preamble() + R"(

OpEntryPoint Fragment %100 "main"
OpExecutionMode %100 OriginUpperLeft
OpDecorate %10 DescriptorSet 0
OpDecorate %10 Binding 0
OpDecorate %20 DescriptorSet 2
OpDecorate %20 Binding 1

%void = OpTypeVoid
%voidfn = OpTypeFunction %void

%bool = OpTypeBool
%true = OpConstantTrue %bool
%float = OpTypeFloat 32

%v2float = OpTypeVector %float 2
%v4float = OpTypeVector %float 4

%v2float_null = OpConstantNull %v2float

%sampler = OpTypeSampler
%ptr_sampler = OpTypePointer UniformConstant %sampler
%im_ty = OpTypeImage %float 1D 0 0 0 1 Unknown
%ptr_im_ty = OpTypePointer UniformConstant %im_ty
%si_ty = OpTypeSampledImage %im_ty

%10 = OpVariable %ptr_sampler UniformConstant
%20 = OpVariable %ptr_im_ty UniformConstant

%100 = OpFunction %void None %voidfn
%entry = OpLabel
%900 = OpCopyObject %v2float %v2float_null        ; definition here
OpSelectionMerge %99 None
OpBranchConditional %true %40 %99

  %40 = OpLabel
  OpSelectionMerge %80 None
  OpBranchConditional %true %50 %80

    %50 = OpLabel
    %sam = OpLoad %sampler %10
    %im = OpLoad %im_ty %20
    %sampled_image = OpSampledImage %si_ty %im %sam
    %result = OpImageSampleImplicitLod %v4float %sampled_image %900 ; usage here
    OpBranch %80

  %80 = OpLabel
  OpBranch %99

%99 = OpLabel
OpReturn
OpFunctionEnd

  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_900 : vec2f;
x_900 = vec2f();
if (true) {
  if (true) {
    let x_19 = textureSample(x_20, x_10, x_900.x);
  }
}
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, TexelTypeWhenLoop) {
    // Demonstrates fix for crbug.com/tint/1642
    // The problem is the texel value for an image write
    // can be given in 'var' declaration.
    //
    // The texel value handling has to unwrap the ref type first.
    const auto assembly = Preamble() + R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %100 "main"
               OpExecutionMode %100 LocalSize 8 8 1
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %Output2Texture2D "Output2Texture2D"
               OpName %100 "main"
               OpDecorate %Output2Texture2D DescriptorSet 0
               OpDecorate %Output2Texture2D Binding 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
          %7 = OpConstantComposite %v2float %float_0 %float_0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
         %12 = OpConstantComposite %v2float %float_1 %float_1
      %int_1 = OpConstant %int 1
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v2uint = OpTypeVector %uint 2
         %17 = OpConstantComposite %v2uint %uint_1 %uint_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 2 Rg32f
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
       %bool = OpTypeBool
%Output2Texture2D = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
        %100 = OpFunction %void None %20
         %22 = OpLabel
               OpBranch %23
         %23 = OpLabel
         %24 = OpPhi %v2float %7 %22 %12 %25
         %26 = OpPhi %int %int_0 %22 %27 %25
         %28 = OpSLessThan %bool %26 %int_2
               OpLoopMerge %29 %25 None
               OpBranchConditional %28 %25 %29
         %25 = OpLabel
         %27 = OpIAdd %int %26 %int_1
               OpBranch %23
         %29 = OpLabel
         %30 = OpLoad %type_2d_image %Output2Texture2D
               OpImageWrite %30 %17 %24 None
               OpReturn
               OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_24 : vec2f;
var x_26 : i32;
x_24 = vec2f();
x_26 = 0i;
loop {
  var x_27 : i32;
  if ((x_26 < 2i)) {
  } else {
    break;
  }

  continuing {
    x_27 = (x_26 + 1i);
    x_24 = vec2f(1.0f);
    x_26 = x_27;
  }
}
textureStore(Output2Texture2D, vec2i(vec2u(1u)), vec4f(x_24, 0.0f, 0.0f));
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, ReadWriteStorageTexture) {
    const auto assembly = Preamble() + R"(
               OpCapability Shader
               OpCapability StorageImageExtendedFormats
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %100 "main"
               OpExecutionMode %100 LocalSize 8 8 1
               OpSource HLSL 600
               OpName %type_2d_image "type.2d.image"
               OpName %RWTexture2D "RWTexture2D"
               OpName %100 "main"
               OpDecorate %RWTexture2D DescriptorSet 0
               OpDecorate %RWTexture2D Binding 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %v4float = OpTypeVector %float 4
       %uint = OpTypeInt 32 0
     %uint_1 = OpConstant %uint 1
     %v2uint = OpTypeVector %uint 2
      %coord = OpConstantComposite %v2uint %uint_1 %uint_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 2 Rgba32f
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
%RWTexture2D = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
        %100 = OpFunction %void None %20
         %22 = OpLabel
         %30 = OpLoad %type_2d_image %RWTexture2D
         %31 = OpImageRead %v4float %30 %coord None
         %32 = OpFAdd %v4float %31 %31
               OpImageWrite %30 %coord %32 None
               OpReturn
               OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << p->error() << assembly;

    EXPECT_TRUE(p->error().empty()) << p->error();
    const auto got = test::ToString(p->program());
    auto* expect =
        R"(requires readonly_and_readwrite_storage_textures;

@group(0) @binding(0) var RWTexture2D : texture_storage_2d<rgba32float, read_write>;

const x_9 = vec2u(1u);

fn main_1() {
  let x_31 = textureLoad(RWTexture2D, vec2i(x_9));
  textureStore(RWTexture2D, vec2i(x_9), (x_31 + x_31));
  return;
}

@compute @workgroup_size(8i, 8i, 1i)
fn main() {
  main_1();
}
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, SimpleSelectCanSelectFromHoistedConstant) {
    // Demonstrates fix for crbug.com/tint/1642
    // The problem is an operand to a simple select can be a value
    // that is hoisted into a 'var' declaration.
    //
    // The selection-generation logic has to UnwrapRef if needed.
    const auto assembly = Preamble() + R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %100 "main" %gl_Position
               OpSource HLSL 600
               OpName %100 "main"
               OpDecorate %gl_Position BuiltIn Position
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
       %bool = OpTypeBool
%gl_Position = OpVariable %_ptr_Output_v4float Output
         %11 = OpUndef %float
        %100 = OpFunction %void None %9
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
         %14 = OpPhi %float %11 %12 %15 %16
         %15 = OpPhi %float %float_0 %12 %17 %16
         %18 = OpFOrdLessThan %bool %15 %float_1
               OpLoopMerge %19 %16 None
               OpBranchConditional %18 %16 %19
         %16 = OpLabel
         %17 = OpFAdd %float %15 %float_1
               OpBranch %13
         %19 = OpLabel
         %20 = OpFOrdGreaterThan %bool %14 %float_1
         %21 = OpSelect %float %20 %14 %float_0
         %22 = OpCompositeConstruct %v4float %21 %21 %21 %21
               OpStore %gl_Position %22
               OpReturn
               OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule()) << assembly;
    auto fe = p->function_emitter(100);
    EXPECT_TRUE(fe.EmitBody()) << p->error();
    EXPECT_TRUE(p->error().empty()) << p->error();
    auto ast_body = fe.ast_body();
    const auto got = test::ToString(p->program(), ast_body);
    auto* expect = R"(var x_14 : f32;
var x_15 : f32;
x_14 = 0.0f;
x_15 = 0.0f;
loop {
  var x_17 : f32;
  if ((x_15 < 1.0f)) {
  } else {
    break;
  }

  continuing {
    x_17 = (x_15 + 1.0f);
    let x_15_c16_1 = x_15;
    x_14 = x_15_c16_1;
    x_15 = x_17;
  }
}
let x_21 = select(0.0f, x_14, (x_14 > 1.0f));
x_1 = vec4f(x_21);
return;
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, OpUndef_FunctionParam) {
    // We can't generate reasonable WGSL when an OpUndef is passed as an argument to a function
    // parameter that is expecting an image object, so just make sure that we do not crash.
    const auto assembly = Preamble() + FragMain() + " " + CommonTypes() + R"(
     %f_ty = OpTypeFunction %void %f_storage_1d
     %20 = OpUndef %f_storage_1d

     %func = OpFunction %void None %f_ty
     %im = OpFunctionParameter %f_storage_1d
     %func_entry = OpLabel
     OpImageWrite %im %uint_1 %v4float_null
     OpReturn
     OpFunctionEnd

     %main = OpFunction %void None %voidfn
     %entry = OpLabel
     %foo = OpFunctionCall %void %func %20
     OpReturn
     OpFunctionEnd
  )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_FALSE(p->BuildAndParseInternalModule());
    EXPECT_EQ(p->error(), "invalid handle object passed as function parameter");
}

TEST_F(SpvParserHandleTest, Image_UnknownDepth_NonDepthUse) {
    const auto assembly = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %7 DescriptorSet 0
               OpDecorate %7 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %4 = OpTypeImage %float 2D 2 0 0 1 Unknown
          %5 = OpTypeSampler
    %v2float = OpTypeVector %float 2
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
          %6 = OpTypeSampledImage %4
%_ptr_UniformConstant_4 = OpTypePointer UniformConstant %4
          %7 = OpVariable %_ptr_UniformConstant_4 UniformConstant
%_ptr_UniformConstant_5 = OpTypePointer UniformConstant %5
          %8 = OpVariable %_ptr_UniformConstant_5 UniformConstant
      %const = OpConstantNull %v2float
          %1 = OpFunction %void None %15
         %18 = OpLabel
         %20 = OpLoad %4 %7
         %21 = OpLoad %5 %8
         %22 = OpSampledImage %6 %20 %21
         %23 = OpImageSampleImplicitLod %v4float %22 %const None
               OpReturn
               OpFunctionEnd
    )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    const auto got = test::ToString(p->program());
    auto* expect = R"(@group(0) @binding(0) var x_7 : texture_2d<f32>;

@group(0) @binding(1) var x_8 : sampler;

fn main_1() {
  let x_23 = textureSample(x_7, x_8, vec2f());
  return;
}

@fragment
fn main() {
  main_1();
}
)";
    ASSERT_EQ(expect, got);
}

TEST_F(SpvParserHandleTest, Image_UnknownDepth_DepthUse) {
    const auto assembly = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %7 DescriptorSet 0
               OpDecorate %7 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %8 Binding 1
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
          %4 = OpTypeImage %float 2D 2 0 0 1 Unknown
          %5 = OpTypeSampler
    %v2float = OpTypeVector %float 2
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
          %6 = OpTypeSampledImage %4
%_ptr_UniformConstant_4 = OpTypePointer UniformConstant %4
          %7 = OpVariable %_ptr_UniformConstant_4 UniformConstant
%_ptr_UniformConstant_5 = OpTypePointer UniformConstant %5
          %8 = OpVariable %_ptr_UniformConstant_5 UniformConstant
      %const = OpConstantNull %v2float
    %float_0 = OpConstantNull %float
          %1 = OpFunction %void None %15
         %18 = OpLabel
         %20 = OpLoad %4 %7
         %21 = OpLoad %5 %8
         %22 = OpSampledImage %6 %20 %21
         %23 = OpImageSampleDrefImplicitLod %v4float %22 %const %float_0 None
               OpReturn
               OpFunctionEnd
    )";
    auto p = parser(test::Assemble(assembly));
    EXPECT_TRUE(p->BuildAndParseInternalModule());
    const auto got = test::ToString(p->program());
    auto* expect = R"(@group(0) @binding(0) var x_7 : texture_depth_2d;

@group(0) @binding(1) var x_8 : sampler_comparison;

fn main_1() {
  let x_23 = textureSampleCompare(x_7, x_8, vec2f(), 0.0f);
  return;
}

@fragment
fn main() {
  main_1();
}
)";
    ASSERT_EQ(expect, got);
}

}  // namespace
}  // namespace tint::spirv::reader::ast_parser
