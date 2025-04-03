// Copyright 2018 The Dawn & Tint Authors
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

#include <sstream>
#include <string>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ShaderModuleValidationTest : public ValidationTest {};

#if TINT_BUILD_SPV_READER
// Test case with a simpler shader that should successfully be created
TEST_F(ShaderModuleValidationTest, CreationSuccess) {
    const char* shader = R"(
                   OpCapability Shader
              %1 = OpExtInstImport "GLSL.std.450"
                   OpMemoryModel Logical GLSL450
                   OpEntryPoint Fragment %main "main" %fragColor
                   OpExecutionMode %main OriginUpperLeft
                   OpSource GLSL 450
                   OpSourceExtension "GL_GOOGLE_cpp_style_line_directive"
                   OpSourceExtension "GL_GOOGLE_include_directive"
                   OpName %main "main"
                   OpName %fragColor "fragColor"
                   OpDecorate %fragColor Location 0
           %void = OpTypeVoid
              %3 = OpTypeFunction %void
          %float = OpTypeFloat 32
        %v4float = OpTypeVector %float 4
    %_ptr_Output_v4float = OpTypePointer Output %v4float
      %fragColor = OpVariable %_ptr_Output_v4float Output
        %float_1 = OpConstant %float 1
        %float_0 = OpConstant %float 0
             %12 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
           %main = OpFunction %void None %3
              %5 = OpLabel
                   OpStore %fragColor %12
                   OpReturn
                   OpFunctionEnd)";

    utils::CreateShaderModuleFromASM(device, shader);
}

// Tint's SPIR-V reader transforms a combined image sampler into two
// variables: an image part and a sampler part.  The sampler part's binding
// number is incremented. This may produce a conflict, which is solved
// by iterating further binding increments.  It's easy to use in simple cases:
// a sampled image variable effectively takes up two binding slots.
TEST_F(ShaderModuleValidationTest, CombinedTextureAndSampler) {
    // SPIR-V ASM produced by glslang for the following fragment shader:
    //
    //   #version 450
    //   layout(set = 0, binding = 0) uniform sampler2D tex;
    //   void main () {}
    //
    // Note that the following defines an interface combined texture/sampler which is not allowed
    // in Dawn / WebGPU.
    //
    //   %8 = OpTypeSampledImage %7
    //   %_ptr_UniformConstant_8 = OpTypePointer UniformConstant %8
    //   %tex = OpVariable %_ptr_UniformConstant_8 UniformConstant
    const char* shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %tex "tex"
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
%_ptr_UniformConstant_8 = OpTypePointer UniformConstant %8
        %tex = OpVariable %_ptr_UniformConstant_8 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    utils::CreateShaderModuleFromASM(device, shader);
}

TEST_F(ShaderModuleValidationTest, ArrayOfCombinedTextureAndSampler) {
    // SPIR-V ASM produced by glslang for the following fragment shader:
    //
    //   #version 450
    //   layout(set = 0, binding = 0) uniform sampler2D tex[2];
    //   void main () {}
    //
    // Dawn/WebGPU does not yet support arrays of sampled images.
    const char* shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %tex "tex"
               OpDecorate %tex Binding 0
               OpDecorate %tex DescriptorSet 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 0 0 1 Unknown
          %8 = OpTypeSampledImage %7
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_8_uint_2 = OpTypeArray %8 %uint_2
%_ptr_UniformConstant__arr_8_uint_2 = OpTypePointer UniformConstant %_arr_8_uint_2
        %tex = OpVariable %_ptr_UniformConstant__arr_8_uint_2 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    ASSERT_DEVICE_ERROR(utils::CreateShaderModuleFromASM(device, shader));
}

// Test that it is not allowed to declare a multisampled-array interface texture.
// TODO(enga): Also test multisampled cube, cube array, and 3D. These have no GLSL keywords.
TEST_F(ShaderModuleValidationTest, MultisampledArrayTexture) {
    // SPIR-V ASM produced by glslang for the following fragment shader:
    //
    //  #version 450
    //  layout(set=0, binding=0) uniform texture2DMSArray tex;
    //  void main () {}}
    //
    // Note that the following defines an interface array multisampled texture which is not allowed
    // in Dawn / WebGPU.
    //
    //  %7 = OpTypeImage %float 2D 0 1 1 1 Unknown
    //  %_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
    //  %tex = OpVariable %_ptr_UniformConstant_7 UniformConstant
    const char* shader = R"(
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 450
               OpName %main "main"
               OpName %tex "tex"
               OpDecorate %tex DescriptorSet 0
               OpDecorate %tex Binding 0
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
          %7 = OpTypeImage %float 2D 0 1 1 1 Unknown
%_ptr_UniformConstant_7 = OpTypePointer UniformConstant %7
        %tex = OpVariable %_ptr_UniformConstant_7 UniformConstant
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpReturn
               OpFunctionEnd
        )";

    ASSERT_DEVICE_ERROR(utils::CreateShaderModuleFromASM(device, shader));
}

const char* kShaderWithNonUniformDerivative = R"(
               OpCapability Shader
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %foo "foo" %x
               OpExecutionMode %foo OriginUpperLeft
               OpDecorate %x Location 0
      %float = OpTypeFloat 32
%_ptr_Input_float = OpTypePointer Input %float
          %x = OpVariable %_ptr_Input_float Input
       %void = OpTypeVoid
    %float_0 = OpConstantNull %float
       %bool = OpTypeBool
  %func_type = OpTypeFunction %void
        %foo = OpFunction %void None %func_type
  %foo_start = OpLabel
    %x_value = OpLoad %float %x
  %condition = OpFOrdGreaterThan %bool %x_value %float_0
               OpSelectionMerge %merge None
               OpBranchConditional %condition %true_branch %merge
%true_branch = OpLabel
     %result = OpDPdx %float %x_value
               OpBranch %merge
      %merge = OpLabel
               OpReturn
               OpFunctionEnd)";

// Test that creating a module with a SPIR-V shader that has a uniformity violation fails when no
// SPIR-V options descriptor is used.
TEST_F(ShaderModuleValidationTest, NonUniformDerivatives_NoOptions) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModuleFromASM(device, kShaderWithNonUniformDerivative));
}

// Test that creating a module with a SPIR-V shader that has a uniformity violation fails when
// passing a SPIR-V options descriptor with the `allowNonUniformDerivatives` flag set to `false`.
TEST_F(ShaderModuleValidationTest, NonUniformDerivatives_FlagSetToFalse) {
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor spirv_options_desc = {};
    spirv_options_desc.allowNonUniformDerivatives = false;
    ASSERT_DEVICE_ERROR(utils::CreateShaderModuleFromASM(device, kShaderWithNonUniformDerivative,
                                                         &spirv_options_desc));
}

// Test that creating a module with a SPIR-V shader that has a uniformity violation succeeds when
// passing a SPIR-V options descriptor with the `allowNonUniformDerivatives` flag set to `true`.
TEST_F(ShaderModuleValidationTest, NonUniformDerivatives_FlagSetToTrue) {
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor spirv_options_desc = {};
    spirv_options_desc.allowNonUniformDerivatives = true;
    utils::CreateShaderModuleFromASM(device, kShaderWithNonUniformDerivative, &spirv_options_desc);
}

#endif  // TINT_BUILD_SPV_READER

// Test that it is invalid to create a shader module with no chained descriptor. (It must be
// WGSL or SPIRV, not empty)
TEST_F(ShaderModuleValidationTest, NoChainedDescriptor) {
    wgpu::ShaderModuleDescriptor desc = {};
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&desc));
}

// Test that it is invalid to create a shader module that uses both the WGSL descriptor and the
// SPIRV descriptor.
TEST_F(ShaderModuleValidationTest, MultipleChainedDescriptor_WgslAndSpirv) {
    uint32_t code = 42;
    wgpu::ShaderModuleDescriptor desc = {};
    wgpu::ShaderSourceSPIRV spirv_desc = {};
    spirv_desc.code = &code;
    spirv_desc.codeSize = 1;
    wgpu::ShaderSourceWGSL wgsl_desc = {};
    wgsl_desc.code = "";
    wgsl_desc.nextInChain = &spirv_desc;
    desc.nextInChain = &wgsl_desc;
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&desc));
}

// Test that it is invalid to create a shader module that uses both the WGSL descriptor and the
// Dawn SPIRV options descriptor.
TEST_F(ShaderModuleValidationTest, MultipleChainedDescriptor_WgslAndDawnSpirvOptions) {
    wgpu::ShaderModuleDescriptor desc = {};
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor spirv_options_desc = {};
    wgpu::ShaderSourceWGSL wgsl_desc = {};
    wgsl_desc.nextInChain = &spirv_options_desc;
    wgsl_desc.code = "";
    desc.nextInChain = &wgsl_desc;
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&desc));
}

// Test that it is invalid to create a shader module that only uses the Dawn SPIRV options
// descriptor without the SPIRV descriptor.
TEST_F(ShaderModuleValidationTest, OnlySpirvOptionsDescriptor) {
    wgpu::ShaderModuleDescriptor desc = {};
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor spirv_options_desc = {};
    desc.nextInChain = &spirv_options_desc;
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&desc));
}

// Test that it is invalid to pass ShaderModuleCompilationOptions if the feature is not enabled.
TEST_F(ShaderModuleValidationTest, ShaderModuleCompilationOptionsNoFeature) {
    wgpu::ShaderModuleDescriptor desc = {};
    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgslDesc.code = "@compute @workgroup_size(1) fn main() {}";

    wgpu::ShaderModuleCompilationOptions compilationOptions = {};
    desc.nextInChain = &wgslDesc;
    wgslDesc.nextInChain = &compilationOptions;
    ASSERT_DEVICE_ERROR(device.CreateShaderModule(&desc),
                        testing::HasSubstr("FeatureName::ShaderModuleCompilationOptions"));
}

// Tests that shader module compilation messages can be queried.
TEST_F(ShaderModuleValidationTest, GetCompilationMessages) {
    // This test works assuming ShaderModule is backed by a native::ShaderModuleBase, which
    // is not the case on the wire.
    DAWN_SKIP_TEST_IF(UsesWire());

    wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) vec4f {
            return vec4f(0.0, 1.0, 0.0, 1.0);
        })");

    native::ShaderModuleBase* shaderModuleBase = native::FromAPI(shaderModule.Get());
    native::OwnedCompilationMessages* messages = shaderModuleBase->GetCompilationMessages();
    messages->ClearMessages();
    messages->AddMessageForTesting("Info Message");
    messages->AddMessageForTesting("Warning Message", wgpu::CompilationMessageType::Warning);
    messages->AddMessageForTesting("Error Message", wgpu::CompilationMessageType::Error, 3, 4);
    messages->AddMessageForTesting("Complete Message", wgpu::CompilationMessageType::Info, 3, 4, 5,
                                   6);

    shaderModule.GetCompilationInfo(
        wgpu::CallbackMode::AllowSpontaneous,
        [](wgpu::CompilationInfoRequestStatus status, const wgpu::CompilationInfo* info) {
            ASSERT_EQ(wgpu::CompilationInfoRequestStatus::Success, status);
            ASSERT_NE(nullptr, info);
            ASSERT_EQ(4u, info->messageCount);

            const wgpu::CompilationMessage* message = &info->messages[0];
            ASSERT_EQ("Info Message", std::string_view(message->message));
            ASSERT_EQ(wgpu::CompilationMessageType::Info, message->type);
            ASSERT_EQ(0u, message->lineNum);
            ASSERT_EQ(0u, message->linePos);
            ASSERT_NE(nullptr, message->nextInChain);
            ASSERT_EQ(wgpu::SType::DawnCompilationMessageUtf16, message->nextInChain->sType);
            const wgpu::DawnCompilationMessageUtf16* utf16 =
                reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(message->nextInChain);
            EXPECT_EQ(0u, utf16->linePos);

            message = &info->messages[1];
            ASSERT_EQ("Warning Message", std::string_view(message->message));
            ASSERT_EQ(wgpu::CompilationMessageType::Warning, message->type);
            ASSERT_EQ(0u, message->lineNum);
            ASSERT_EQ(0u, message->linePos);
            ASSERT_NE(nullptr, message->nextInChain);
            ASSERT_EQ(wgpu::SType::DawnCompilationMessageUtf16, message->nextInChain->sType);
            utf16 =
                reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(message->nextInChain);
            EXPECT_EQ(0u, utf16->linePos);

            message = &info->messages[2];
            ASSERT_EQ("Error Message", std::string_view(message->message));
            ASSERT_EQ(wgpu::CompilationMessageType::Error, message->type);
            ASSERT_EQ(3u, message->lineNum);
            ASSERT_EQ(4u, message->linePos);
            ASSERT_NE(nullptr, message->nextInChain);
            ASSERT_EQ(wgpu::SType::DawnCompilationMessageUtf16, message->nextInChain->sType);
            utf16 =
                reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(message->nextInChain);
            EXPECT_EQ(4u, utf16->linePos);

            message = &info->messages[3];
            ASSERT_EQ("Complete Message", std::string_view(message->message));
            ASSERT_EQ(wgpu::CompilationMessageType::Info, message->type);
            ASSERT_EQ(3u, message->lineNum);
            ASSERT_EQ(4u, message->linePos);
            ASSERT_EQ(5u, message->offset);
            ASSERT_EQ(6u, message->length);
            ASSERT_NE(nullptr, message->nextInChain);
            ASSERT_EQ(wgpu::SType::DawnCompilationMessageUtf16, message->nextInChain->sType);
            utf16 =
                reinterpret_cast<const wgpu::DawnCompilationMessageUtf16*>(message->nextInChain);
            EXPECT_EQ(4u, utf16->linePos);
            ASSERT_EQ(5u, utf16->offset);
            ASSERT_EQ(6u, utf16->length);
        });
}

// Validate the maximum location of effective inter-stage variables cannot be greater than 16
// (kMaxInterStageShaderVariables).
TEST_F(ShaderModuleValidationTest, MaximumShaderIOLocations) {
    auto CheckTestPipeline = [&](bool success, uint32_t maximumOutputLocation,
                                 wgpu::ShaderStage failingShaderStage) {
        // Build the ShaderIO struct containing variables up to maximumOutputLocation.
        std::ostringstream stream;
        stream << "struct ShaderIO {\n";
        for (uint32_t location = 1; location <= maximumOutputLocation; ++location) {
            stream << "@location(" << location << ") var" << location << ": f32,\n";
        }

        if (failingShaderStage == wgpu::ShaderStage::Vertex) {
            stream << " @builtin(position) pos: vec4f,";
        }
        stream << "}\n";

        std::string ioStruct = stream.str();

        // Build the test pipeline. Note that it's not possible with just ASSERT_DEVICE_ERROR
        // whether it is the vertex or fragment shader that fails. So instead we will look for the
        // string "failingVertex" or "failingFragment" in the error message.
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

        const char* errorMatcher = nullptr;
        switch (failingShaderStage) {
            case wgpu::ShaderStage::Vertex: {
                errorMatcher = "failingVertex";
                pDesc.vertex.entryPoint = "failingVertex";
                pDesc.vertex.module = utils::CreateShaderModule(device, (ioStruct + R"(
                    @vertex fn failingVertex() -> ShaderIO {
                        var shaderIO : ShaderIO;
                        shaderIO.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                        return shaderIO;
                     }
                )")
                                                                            .c_str());
                pDesc.cFragment.module = utils::CreateShaderModule(device, R"(
                    @fragment fn main() -> @location(0) vec4f {
                        return vec4f(0.0);
                    }
                )");
                break;
            }

            case wgpu::ShaderStage::Fragment: {
                errorMatcher = "failingFragment";
                pDesc.cFragment.entryPoint = "failingFragment";
                pDesc.cFragment.module = utils::CreateShaderModule(device, (ioStruct + R"(
                    @fragment fn failingFragment(io : ShaderIO) -> @location(0) vec4f {
                        return vec4f(0.0);
                     }
                )")
                                                                               .c_str());
                pDesc.vertex.module = utils::CreateShaderModule(device, R"(
                    @vertex fn main() -> @builtin(position) vec4f {
                        return vec4f(0.0);
                    }
                )");
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }

        if (success) {
            if (failingShaderStage == wgpu::ShaderStage::Vertex) {
                // It is allowed that fragment inputs are a subset of the vertex output variables.
                device.CreateRenderPipeline(&pDesc);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                    testing::HasSubstr("The fragment input at location"));
            }
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                testing::HasSubstr(errorMatcher));
        }
    };

    // It is allowed to create a shader module with the maximum active vertex output location ==
    // (kMaxInterStageShaderVariables - 1);
    CheckTestPipeline(true, kMaxInterStageShaderVariables - 1, wgpu::ShaderStage::Vertex);

    // It isn't allowed to create a shader module with the maximum active vertex output location ==
    // kMaxInterStageShaderVariables;
    CheckTestPipeline(false, kMaxInterStageShaderVariables, wgpu::ShaderStage::Vertex);

    // It is allowed to create a shader module with the maximum active fragment input location ==
    // (kMaxInterStageShaderVariables - 1);
    CheckTestPipeline(true, kMaxInterStageShaderVariables - 1, wgpu::ShaderStage::Fragment);

    // It isn't allowed to create a shader module with the maximum active vertex output location ==
    // kMaxInterStageShaderVariables;
    CheckTestPipeline(false, kMaxInterStageShaderVariables, wgpu::ShaderStage::Fragment);
}

// Validate the number of total inter-stage user-defined variables count and built-in variables
// cannot exceed kMaxInterStageShaderVariables.
TEST_F(ShaderModuleValidationTest, MaximumInterStageShaderVariables) {
    auto CheckTestPipeline = [&](bool success,
                                 uint32_t totalUserDefinedInterStageShaderVariablesCount,
                                 wgpu::ShaderStage failingShaderStage,
                                 const char* extraBuiltInDeclarations = "",
                                 bool usePointListAsPrimitiveType = false) {
        // Build the ShaderIO struct containing totalUserDefinedInterStageShaderVariablesCount
        // variables.
        std::ostringstream stream;
        stream << "struct ShaderIO {\n" << extraBuiltInDeclarations << "\n";
        uint32_t vec4InputLocations = totalUserDefinedInterStageShaderVariablesCount;

        for (uint32_t location = 0; location < vec4InputLocations; ++location) {
            stream << "@location(" << location << ") var" << location << ": vec4f,\n";
        }

        if (failingShaderStage == wgpu::ShaderStage::Vertex) {
            stream << " @builtin(position) pos: vec4f,\n";
        }
        stream << "}\n";

        std::string ioStruct = stream.str();

        // Build the test pipeline. Note that it's not possible with just ASSERT_DEVICE_ERROR
        // whether it is the vertex or fragment shader that fails. So instead we will look for the
        // string "failingVertex" or "failingFragment" in the error message.
        utils::ComboRenderPipelineDescriptor pDesc;
        pDesc.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;
        if (usePointListAsPrimitiveType) {
            pDesc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        } else {
            pDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
        }

        const char* errorMatcher = nullptr;
        switch (failingShaderStage) {
            case wgpu::ShaderStage::Vertex: {
                if (usePointListAsPrimitiveType) {
                    errorMatcher = "PointList";
                } else {
                    errorMatcher = "failingVertex";
                }

                std::string shader = ioStruct + R"(
                    @vertex fn failingVertex() -> ShaderIO {
                        var shaderIO : ShaderIO;
                        shaderIO.pos = vec4f(0.0, 0.0, 0.0, 1.0);
                        return shaderIO;
                     }
                    @fragment fn main() -> @location(0) vec4f {
                        return vec4f(0.0);
                    })";
                wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader);

                pDesc.vertex.entryPoint = "failingVertex";
                pDesc.vertex.module = shaderModule;
                pDesc.cFragment.module = shaderModule;
                break;
            }

            case wgpu::ShaderStage::Fragment: {
                std::string shader = ioStruct + R"(
                     @vertex fn main() -> @builtin(position) vec4f {
                        return vec4f(0.0);
                     }
                     @fragment fn failingFragment(io : ShaderIO) -> @location(0) vec4f {
                        return vec4f(0.0);
                     })";
                wgpu::ShaderModule shaderModule = utils::CreateShaderModule(device, shader);

                errorMatcher = "failingFragment";
                pDesc.cFragment.entryPoint = "failingFragment";
                pDesc.cFragment.module = shaderModule;
                pDesc.vertex.module = shaderModule;
                break;
            }

            default:
                DAWN_UNREACHABLE();
        }

        if (success) {
            if (failingShaderStage == wgpu::ShaderStage::Vertex) {
                // It is allowed that fragment inputs are a subset of the vertex output variables.
                device.CreateRenderPipeline(&pDesc);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                    testing::HasSubstr("The fragment input at location"));
            }
        } else {
            ASSERT_DEVICE_ERROR(device.CreateRenderPipeline(&pDesc),
                                testing::HasSubstr(errorMatcher));
        }
    };

    // Verify when there is no input builtin variable in a fragment shader, the total user-defined
    // input variables count must be less than kMaxInterStageShaderVariables.
    {
        CheckTestPipeline(true, kMaxInterStageShaderVariables, wgpu::ShaderStage::Fragment);
        CheckTestPipeline(false, kMaxInterStageShaderVariables + 1, wgpu::ShaderStage::Fragment);
    }

    // Verify the total user-defined vertex output variables count must be less than
    // kMaxInterStageShaderVariables.
    {
        CheckTestPipeline(true, kMaxInterStageShaderVariables, wgpu::ShaderStage::Vertex);
        CheckTestPipeline(false, kMaxInterStageShaderVariables + 1, wgpu::ShaderStage::Vertex);
    }

    // Verify the total user-defined vertex output variables count must be less than or equal to
    // (kMaxInterStageShaderVariables - 1) when the primitive topology is PointList.
    {
        constexpr bool kUsePointListAsPrimitiveTopology = true;
        const char* kExtraBuiltins = "";

        {
            uint32_t variablesCount = kMaxInterStageShaderVariables - 1;
            CheckTestPipeline(true, variablesCount, wgpu::ShaderStage::Vertex, kExtraBuiltins,
                              kUsePointListAsPrimitiveTopology);
        }
        {
            uint32_t variablesCount = kMaxInterStageShaderVariables;
            CheckTestPipeline(false, variablesCount, wgpu::ShaderStage::Vertex, kExtraBuiltins,
                              kUsePointListAsPrimitiveTopology);
        }
    }

    // @builtin(position) in fragment shaders shouldn't be counted into the maximum inter-stage
    // variables count.
    {
        CheckTestPipeline(true, kMaxInterStageShaderVariables, wgpu::ShaderStage::Fragment,
                          "@builtin(position) fragCoord : vec4f,");
    }

    // @builtin(front_facing), @builtin(sample_index) and @builtin(sample_mask) should all be
    // counted into the maximum inter-stage variables count. Then the maximum user-defined
    // inter-stage shader variables can only be (kMaxInterStageShaderVariables - 1) because these
    // user-defined inter-stage shader variables always consume 1 shader variable.
    {
        constexpr uint8_t kMaskFrontFacing = 1;
        constexpr uint8_t kMaskSampleIndex = 1 << 1;
        constexpr uint8_t kMaskSampleMask = 1 << 2;
        for (uint8_t mask = 1; mask <= 7; ++mask) {
            std::string builtInDeclarations = "";
            if (mask & kMaskFrontFacing) {
                builtInDeclarations += "@builtin(front_facing) frontFacing : bool,";
            }
            if (mask & kMaskSampleIndex) {
                builtInDeclarations += "@builtin(sample_index) sampleIndex : u32,";
            }
            if (mask & kMaskSampleMask) {
                builtInDeclarations += "@builtin(sample_mask) sampleMask : u32,";
            }

            {
                uint32_t variablesCount = kMaxInterStageShaderVariables - 1;
                CheckTestPipeline(true, variablesCount, wgpu::ShaderStage::Fragment,
                                  builtInDeclarations.c_str());
            }
            {
                uint32_t variablesCount = kMaxInterStageShaderVariables;
                CheckTestPipeline(false, variablesCount, wgpu::ShaderStage::Fragment,
                                  builtInDeclarations.c_str());
            }
        }
    }
}

// Test that numeric ID must be unique
TEST_F(ShaderModuleValidationTest, OverridableConstantsNumericIDConflicts) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
@id(1234) override c0: u32;
@id(1234) override c1: u32;

struct Buf {
    data : array<u32, 2>
}

@group(0) @binding(0) var<storage, read_write> buf : Buf;

@compute @workgroup_size(1) fn main() {
    // make sure the overridable constants are not optimized out
    buf.data[0] = c0;
    buf.data[1] = c1;
})"));
}

// Test that @binding must be less then kMaxBindingsPerBindGroup
TEST_F(ShaderModuleValidationTest, MaxBindingNumber) {
    static_assert(kMaxBindingsPerBindGroup == 1000);

    wgpu::ComputePipelineDescriptor desc;

    // kMaxBindingsPerBindGroup-1 is valid.
    desc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(999) var s : sampler;
        @compute @workgroup_size(1) fn main() {
            _ = s;
        }
    )");
    device.CreateComputePipeline(&desc);

    // kMaxBindingsPerBindGroup is an error
    desc.compute.module = utils::CreateShaderModule(device, R"(
        @group(0) @binding(1000) var s : sampler;
        @compute @workgroup_size(1) fn main() {
            _ = s;
        }
    )");
    ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&desc));
}

// Test that missing decorations on shader IO or bindings causes a validation error.
TEST_F(ShaderModuleValidationTest, MissingDecorations) {
    // Vertex input.
    utils::CreateShaderModule(device, R"(
        @vertex fn main(@location(0) a : vec4f) -> @builtin(position) vec4f {
            return vec4(1.0);
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @vertex fn main(a : vec4f) -> @builtin(position) vec4f {
            return vec4(1.0);
        }
    )"));

    // Vertex output
    utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(position) pos : vec4f,
            @location(0) a : f32,
        }
        @vertex fn main() -> Output {
            var output : Output;
            return output;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        struct Output {
            @builtin(position) pos : vec4f,
            a : f32,
        }
        @vertex fn main() -> Output {
            var output : Output;
            return output;
        }
    )"));

    // Fragment input
    utils::CreateShaderModule(device, R"(
        @fragment fn main(@location(0) a : vec4f) -> @location(0) f32 {
            return 1.0;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @fragment fn main(a : vec4f) -> @location(0) f32 {
            return 1.0;
        }
    )"));

    // Fragment input
    utils::CreateShaderModule(device, R"(
        @fragment fn main() -> @location(0) f32 {
            return 1.0;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @fragment fn main() -> f32 {
            return 1.0;
        }
    )"));

    // Binding decorations
    utils::CreateShaderModule(device, R"(
        @group(0) @binding(0) var s : sampler;
        @fragment fn main() -> @location(0) f32 {
            _ = s;
            return 1.0;
        }
    )");
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @binding(0) var s : sampler;
        @fragment fn main() -> @location(0) f32 {
            _ = s;
            return 1.0;
        }
    )"));
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        @group(0) var s : sampler;
        @fragment fn main() -> @location(0) f32 {
            _ = s;
            return 1.0;
        }
    )"));
}

// Test creating an error shader module with device.CreateErrorShaderModule()
TEST_F(ShaderModuleValidationTest, CreateErrorShaderModule) {
    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgpu::ShaderModuleDescriptor descriptor = {};
    descriptor.nextInChain = &wgslDesc;
    wgslDesc.code = "@compute @workgroup_size(1) fn main() {}";

    wgpu::ShaderModule errorShaderModule;
    ASSERT_DEVICE_ERROR(errorShaderModule = device.CreateErrorShaderModule(
                            &descriptor, "Shader compilation error"));

    errorShaderModule.GetCompilationInfo(
        wgpu::CallbackMode::AllowSpontaneous,
        [](wgpu::CompilationInfoRequestStatus status, const wgpu::CompilationInfo* info) {
            ASSERT_EQ(wgpu::CompilationInfoRequestStatus::Success, status);
            ASSERT_NE(nullptr, info);
            ASSERT_EQ(1u, info->messageCount);

            const wgpu::CompilationMessage* message = &info->messages[0];
            ASSERT_EQ("Shader compilation error", std::string_view(message->message));
            ASSERT_EQ(wgpu::CompilationMessageType::Error, message->type);
            ASSERT_EQ(0u, message->lineNum);
            ASSERT_EQ(0u, message->linePos);
        });

    FlushWire();
}

struct WGSLExtensionInfo {
    const char* wgslName;
    // Is this WGSL extension experimental, i.e. guarded by AllowUnsafeAPIs toggle
    bool isExperimental;
    // The WebGPU features that required to enable this extension, set to empty if no feature
    // required.
    const std::vector<const char*> requiredFeatureNames;
    // The WGSL extensions dependency that required to enable this extension, set to empty if no
    // dependency.
    const std::vector<const char*> dependingExtensionNames;
};

const struct WGSLExtensionInfo kExtensions[] = {
    {"f16", false, {"shader-f16"}, {}},
    {"clip_distances", false, {"clip-distances"}, {}},
    {"dual_source_blending", false, {"dual-source-blending"}, {}},
    {"subgroups", false, {"subgroups"}, {}},
    {"subgroups_f16", false, {"shader-f16", "subgroups", "subgroups-f16"}, {"f16", "subgroups"}},
    {"chromium_experimental_pixel_local", true, {"pixel-local-storage-coherent"}, {}},
    {"chromium_disable_uniformity_analysis", true, {}, {}},
    {"chromium_internal_graphite", true, {}, {}},
    {"chromium_experimental_framebuffer_fetch", true, {"framebuffer-fetch"}, {}},
    {"chromium_experimental_subgroup_matrix", true, {"chromium-experimental-subgroup-matrix"}, {}},

    // Currently the following WGSL extensions are not enabled under any situation.
    /*
    {"chromium_experimental_push_constant", true, {}},
    {"chromium_internal_relaxed_uniform_layout", true, {}},
    */
};

std::string EnableDependingWGSLExtensions(const WGSLExtensionInfo& extension) {
    std::stringstream s;
    for (const char* dependency : extension.dependingExtensionNames) {
        s << "enable " << dependency << ";\n";
    }
    return s.str();
}

class ShaderModuleExtensionValidationTestBase : public ValidationTest {
  protected:
    // Skip tests if using Wire, because some features are not supported by the wire and cause the
    // device creation failed.
    void SetUp() override {
        DAWN_SKIP_TEST_IF(UsesWire());
        ValidationTest::SetUp();
    }

    std::vector<wgpu::FeatureName> GetAllFeatures() {
        std::vector<wgpu::FeatureName> requiredFeatures;
        wgpu::SupportedFeatures supportedFeatures;
        adapter.GetFeatures(&supportedFeatures);
        for (uint32_t i = 0; i < supportedFeatures.featureCount; ++i) {
            requiredFeatures.push_back(supportedFeatures.features[i]);
        }
        return requiredFeatures;
    }
};

// Test validating WGSL extension on safe device with no feature required.
class ShaderModuleExtensionValidationTestSafeNoFeature
    : public ShaderModuleExtensionValidationTestBase {
  protected:
    bool AllowUnsafeAPIs() override { return false; }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

TEST_F(ShaderModuleExtensionValidationTestSafeNoFeature,
       OnlyStableExtensionsRequiringNoFeatureAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnableDependingWGSLExtensions(extension) + std::string("enable ") +
                           extension.wgslName + R"(;

@compute @workgroup_size(1) fn main() {})";

        // On a safe device with no feature required, only stable extensions requiring no features
        // are allowed.
        if (!extension.isExperimental && extension.requiredFeatureNames.size() == 0) {
            utils::CreateShaderModule(device, wgsl.c_str());
        } else {
            ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
        }
    }
}

// Test validating WGSL extension on unsafe device with no feature required.
class ShaderModuleExtensionValidationTestUnsafeNoFeature
    : public ShaderModuleExtensionValidationTestBase {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

TEST_F(ShaderModuleExtensionValidationTestUnsafeNoFeature,
       OnlyExtensionsRequiringNoFeatureAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnableDependingWGSLExtensions(extension) + std::string("enable ") +
                           extension.wgslName + R"(;

@compute @workgroup_size(1) fn main() {})";

        // On an unsafe device with no feature required, only extensions requiring no features are
        // allowed.
        if (extension.requiredFeatureNames.size() == 0) {
            utils::CreateShaderModule(device, wgsl.c_str());
        } else {
            ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
        }
    }
}

// Test validating WGSL extension on safe device with all features required.
class ShaderModuleExtensionValidationTestSafeAllFeatures
    : public ShaderModuleExtensionValidationTestBase {
  protected:
    bool AllowUnsafeAPIs() override { return false; }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return GetAllFeatures(); }
};

TEST_F(ShaderModuleExtensionValidationTestSafeAllFeatures, OnlyStableExtensionsAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnableDependingWGSLExtensions(extension) + std::string("enable ") +
                           extension.wgslName + R"(;

@compute @workgroup_size(1) fn main() {})";

        // On a safe device with all feature required, only stable extensions are allowed.
        if (!extension.isExperimental) {
            utils::CreateShaderModule(device, wgsl.c_str());
        } else {
            ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
        }
    }
}

// Test validating WGSL extension on unsafe device with all features required.
class ShaderModuleExtensionValidationTestUnsafeAllFeatures
    : public ShaderModuleExtensionValidationTestBase {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return GetAllFeatures(); }
};

TEST_F(ShaderModuleExtensionValidationTestUnsafeAllFeatures, AllExtensionsAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnableDependingWGSLExtensions(extension) + std::string("enable ") +
                           extension.wgslName + R"(;

@compute @workgroup_size(1) fn main() {})";

        // On an unsafe device with all feature required, all extensions are allowed.
        utils::CreateShaderModule(device, wgsl.c_str());
    }
}

// Test it is valid to chain ShaderModuleCompilationOptions and path true/false for strictMath.
TEST_F(ShaderModuleExtensionValidationTestUnsafeAllFeatures, ShaderModuleCompilationOptions) {
    wgpu::ShaderModuleDescriptor desc = {};
    wgpu::ShaderSourceWGSL wgslDesc = {};
    wgslDesc.code = "@compute @workgroup_size(1) fn main() {}";

    wgpu::ShaderModuleCompilationOptions compilationOptions = {};
    desc.nextInChain = &wgslDesc;
    wgslDesc.nextInChain = &compilationOptions;

    compilationOptions.strictMath = false;
    device.CreateShaderModule(&desc);

    compilationOptions.strictMath = true;
    device.CreateShaderModule(&desc);
}

}  // anonymous namespace
}  // namespace dawn
