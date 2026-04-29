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

#include <bit>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "dawn/common/Constants.h"
#include "dawn/native/CompilationMessages.h"
#include "dawn/native/ShaderModule.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

#if TINT_BUILD_SPV_READER && !defined(__EMSCRIPTEN__)
#include "spirv-tools/optimizer.hpp"
#endif  // TINT_BUILD_SPV_READER && !defined(__EMSCRIPTEN__)

namespace dawn {
namespace {

class ShaderModuleValidationTest : public ValidationTest {};

#if TINT_BUILD_SPV_READER && !defined(__EMSCRIPTEN__)

wgpu::ShaderModule CreateShaderModuleFromASM(
    const wgpu::Device& device,
    const char* source,
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor* spirv_options = nullptr) {
    // Use SPIRV-Tools's C API to assemble the SPIR-V assembly text to binary. Because the types
    // aren't RAII, we don't return directly on success and instead always go through the code
    // path that destroys the SPIRV-Tools objects.
    wgpu::ShaderModule result = nullptr;

    spv_context context = spvContextCreate(SPV_ENV_UNIVERSAL_1_3);
    DAWN_ASSERT(context != nullptr);

    spv_binary spirv = nullptr;
    spv_diagnostic diagnostic = nullptr;
    if (spvTextToBinary(context, source, strlen(source), &spirv, &diagnostic) == SPV_SUCCESS) {
        DAWN_ASSERT(spirv != nullptr);
        DAWN_ASSERT(spirv->wordCount <= std::numeric_limits<uint32_t>::max());

        wgpu::ShaderSourceSPIRV spirvDesc;
        spirvDesc.codeSize = static_cast<uint32_t>(spirv->wordCount);
        spirvDesc.code = spirv->code;
        spirvDesc.nextInChain = spirv_options;

        wgpu::ShaderModuleDescriptor descriptor;
        descriptor.nextInChain = &spirvDesc;
        result = device.CreateShaderModule(&descriptor);
    } else {
        DAWN_ASSERT(diagnostic != nullptr);
        dawn::WarningLog() << "CreateShaderModuleFromASM SPIRV assembly error:"
                           << diagnostic->position.line + 1 << ":"
                           << diagnostic->position.column + 1 << ": " << diagnostic->error;
    }

    spvDiagnosticDestroy(diagnostic);
    spvBinaryDestroy(spirv);
    spvContextDestroy(context);

    return result;
}

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

    CreateShaderModuleFromASM(device, shader);
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

    CreateShaderModuleFromASM(device, shader);
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

    ASSERT_DEVICE_ERROR(CreateShaderModuleFromASM(device, shader));
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

    ASSERT_DEVICE_ERROR(CreateShaderModuleFromASM(device, shader));
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
    ASSERT_DEVICE_ERROR(CreateShaderModuleFromASM(device, kShaderWithNonUniformDerivative));
}

// Test that creating a module with a SPIR-V shader that has a uniformity violation fails when
// passing a SPIR-V options descriptor with the `allowNonUniformDerivatives` flag set to `false`.
TEST_F(ShaderModuleValidationTest, NonUniformDerivatives_FlagSetToFalse) {
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor spirv_options_desc = {};
    spirv_options_desc.allowNonUniformDerivatives = false;
    ASSERT_DEVICE_ERROR(
        CreateShaderModuleFromASM(device, kShaderWithNonUniformDerivative, &spirv_options_desc));
}

// Test that creating a module with a SPIR-V shader that has a uniformity violation succeeds when
// passing a SPIR-V options descriptor with the `allowNonUniformDerivatives` flag set to `true`.
TEST_F(ShaderModuleValidationTest, NonUniformDerivatives_FlagSetToTrue) {
    wgpu::DawnShaderModuleSPIRVOptionsDescriptor spirv_options_desc = {};
    spirv_options_desc.allowNonUniformDerivatives = true;
    CreateShaderModuleFromASM(device, kShaderWithNonUniformDerivative, &spirv_options_desc);
}

#endif  // TINT_BUILD_SPV_READER && !defined(__EMSCRIPTEN__)

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

    // Build a list of messages to test.
    native::ParsedCompilationMessages messages;
    messages.AddMessageForTesting("Info Message");
    messages.AddMessageForTesting("Warning Message", wgpu::CompilationMessageType::Warning);
    messages.AddMessageForTesting("Error Message", wgpu::CompilationMessageType::Error, 3, 4);
    messages.AddMessageForTesting("Complete Message", wgpu::CompilationMessageType::Info, 3, 4, 5,
                                  6);
    auto ownedMessages = std::make_unique<native::OwnedCompilationMessages>(std::move(messages));
    // Set the messages on the shader module base.
    shaderModuleBase->SetCompilationMessagesForTesting(&ownedMessages);
    // Assert that the messages are set.
    ASSERT_EQ(ownedMessages, nullptr);

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

// Test that creating shader modules with invalid UTF-8 is an error.
TEST_F(ShaderModuleValidationTest, UnicodeValidity) {
    // Referenced from src/tint/utils/text/unicode_test.cc
    constexpr std::array<const char*, 14> kValidTestCases = {{
        "", "abc", "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xb8\x96\xe7\x95\x8c",
        "def\xf0\x9f\x91\x8b\xf0\x9f\x8c\x8e",
        "\xed\x9f\xbf",      // CodePoint == 0xD7FF
        "\xed\x9f\xbe",      // CodePoint == 0xD7FF - 1
        "\xee\x80\x80",      // CodePoint == 0xE000
        "\xee\x80\x81",      // CodePoint == 0xE000 + 1
        "\xef\xbf\xbf",      // CodePoint == 0xFFFF
        "\xef\xbf\xbe",      // CodePoint == 0xFFFF - 1
        "\xf0\x90\x80\x80",  // CodePoint == 0x10000
        "\xf0\x90\x80\x81",  // CodePoint == 0x10000 + 1

        // Surrogates are technically invalid code points but most software supports them (including
        // Tint). WGSL coming from JS should never contain surrogates because the JS strings are
        // valid UTF-16.
        "\xed\xa0\x80",  // CodePoint == 0xD7FF + 1
        "\xed\xbf\xbf",  // CodePoint == 0xE000 - 1
    }};
    constexpr std::array<const char*, 9> kErrorTestCases = {{
        "\xd0",              // 2-bytes, missing second byte
        "\xe8\x8f",          // 3-bytes, missing third byte
        "\xf4\x8f\x8f",      // 4-bytes, missing fourth byte
        "\xd0\x7f",          // 2-bytes, second byte MSB unset
        "\xe8\x7f\x8f",      // 3-bytes, second byte MSB unset
        "\xe8\x8f\x7f",      // 3-bytes, third byte MSB unset
        "\xf4\x7f\x8f\x8f",  // 4-bytes, second byte MSB unset
        "\xf4\x8f\x7f\x8f",  // 4-bytes, third byte MSB unset
        "\xf4\x8f\x8f\x7f",  // 4-bytes, fourth byte MSB unset
    }};

    // Puts the UTF-8 in a comment as that's where arbitrary (valid) UTF-8 is allowed.
    const std::string kPrefix = "@compute @workgroup_size(1) fn main () {} \n //";

    for (const char* testCase : kValidTestCases) {
        utils::CreateShaderModule(device, kPrefix + testCase);
    }
    for (const char* testCase : kErrorTestCases) {
        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, kPrefix + testCase));
    }
}

// Validate the number of total inter-stage user-defined variables count and built-in variables
// cannot exceed kMaxInterStageShaderVariables.
class ShaderModuleMaxInterStageShaderVariablesValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        wgpu::SupportedFeatures supportedFeatures;
        adapter.GetFeatures(&supportedFeatures);
        std::vector<wgpu::FeatureName> requiredFeatures(
            supportedFeatures.features,
            supportedFeatures.features + supportedFeatures.featureCount);
        return requiredFeatures;
    }
};

TEST_F(ShaderModuleMaxInterStageShaderVariablesValidationTest, Test) {
    auto CheckTestPipeline =
        [&](bool success, uint32_t totalUserDefinedInterStageShaderVariablesCount,
            wgpu::ShaderStage failingShaderStage, const char* extraBuiltInDeclarations = "",
            bool usePointListAsPrimitiveType = false) {
            std::ostringstream stream;

            // add enables
            if (device.HasFeature(wgpu::FeatureName::PrimitiveIndex)) {
                stream << "enable primitive_index;";
            }

            if (device.HasFeature(wgpu::FeatureName::Subgroups)) {
                stream << "enable subgroups;";
            }

            // Build the ShaderIO struct containing totalUserDefinedInterStageShaderVariablesCount
            // variables.
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
            // whether it is the vertex or fragment shader that fails. So instead we will look for
            // the string "failingVertex" or "failingFragment" in the error message.
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
                    // It is allowed that fragment inputs are a subset of the vertex output
                    // variables.
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

    // @builtin(front_facing), @builtin(sample_index), @builtin(sample_mask),
    // @builtin(primitive_index), @builtin(subgroup_invocation_id) and
    // @builtin(subgroup_size) should all be counted into the maximum
    // inter-stage variables count. Then the maximum user-defined inter-stage
    // shader variables can only be (kMaxInterStageShaderVariables - 1) because
    // these user-defined inter-stage shader variables always consume 1 shader
    // variable each.
    {
        struct Builtin {
            const char* name;
            const char* type;
            const char* extension;
            std::optional<wgpu::FeatureName> requiredFeature;
        };
        Builtin builtins[] = {
            {"front_facing", "bool", nullptr, {}},
            {"sample_index", "u32", nullptr, {}},
            {"sample_mask", "u32", nullptr, {}},
            {"primitive_index", "u32", "primitive_index", wgpu::FeatureName::PrimitiveIndex},
            {"subgroup_invocation_id", "u32", "subgroups", wgpu::FeatureName::Subgroups},
            {"subgroup_size", "u32", "subgroups", wgpu::FeatureName::Subgroups},
        };
        for (uint8_t mask = 1; mask < 1 << std::size(builtins); ++mask) {
            std::string builtInDeclarations = "";
            bool canTest = true;
            for (uint8_t b = 0; b < std::size(builtins); ++b) {
                if (mask & (1 << b)) {
                    const Builtin& builtin = builtins[b];
                    builtInDeclarations += "@builtin(" + std::string(builtin.name) + ") b_" +
                                           std::string(builtin.name) + ": " +
                                           std::string(builtin.type) + ",";
                    if (builtin.requiredFeature.has_value()) {
                        if (!device.HasFeature(builtin.requiredFeature.value())) {
                            canTest = false;
                        }
                    }
                }
            }
            if (canTest) {
                uint32_t variablesCount = kMaxInterStageShaderVariables - std::popcount(mask);
                CheckTestPipeline(true, variablesCount, wgpu::ShaderStage::Fragment,
                                  builtInDeclarations.c_str());
            }
            if (canTest) {
                uint32_t variablesCount = kMaxInterStageShaderVariables - std::popcount(mask) + 1;
                CheckTestPipeline(false, variablesCount, wgpu::ShaderStage::Fragment,
                                  builtInDeclarations.c_str());
            }
        }
    }
}

struct WGSLExtensionInfo {
    const char* wgslName;
    // Is this WGSL extension experimental, i.e. guarded by AllowUnsafeAPIs toggle
    bool isExperimental;
    // The WebGPU features required to enable this extension, set to empty if no feature
    // required.
    const std::vector<wgpu::FeatureName> requiredFeatureNames;
    // The WGSL extensions dependency required to enable this extension, set to empty if no
    // dependency.
    const std::vector<const char*> dependingExtensionNames;
};
std::ostream& operator<<(std::ostream& os, const WGSLExtensionInfo& info) {
    return os << "WGSLExtensionInfo{wgslName: " << info.wgslName
              << ", isExperimental: " << info.isExperimental
              << ", requiredFeatureNames size: " << info.requiredFeatureNames.size()
              << ", dependingExtensionNames size: " << info.dependingExtensionNames.size() << "}";
}

// clang-format off
const WGSLExtensionInfo kExtensions[] = {
    {"f16", false, {wgpu::FeatureName::ShaderF16}, {}},
    {"clip_distances", false, {wgpu::FeatureName::ClipDistances}, {}},
    {"dual_source_blending", false, {wgpu::FeatureName::DualSourceBlending}, {}},
    {"subgroups", false, {wgpu::FeatureName::Subgroups}, {}},
    {"primitive_index", false, {wgpu::FeatureName::PrimitiveIndex}, {}},
    {"chromium_experimental_pixel_local", true, {wgpu::FeatureName::PixelLocalStorageCoherent}, {}},
    {"chromium_disable_uniformity_analysis", true, {}, {}},
    {"chromium_experimental_framebuffer_fetch", true, {wgpu::FeatureName::FramebufferFetch}, {}},
    {"chromium_experimental_subgroup_matrix", true, {wgpu::FeatureName::ChromiumExperimentalSubgroupMatrix}, {}},
    {"chromium_experimental_resource_table", true, {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable}, {}},
    {"chromium_experimental_subgroup_size_control", true, {wgpu::FeatureName::ChromiumExperimentalSubgroupSizeControl}, {"subgroups"}},
    {"atomic_vec2u_min_max", true, {wgpu::FeatureName::AtomicVec2uMinMax}, {}}

    // Currently the following WGSL extensions are not enabled under any situation.
    /*
    {"chromium_internal_relaxed_uniform_layout", true, {}},
    */
};
// clang-format on

class ShaderModuleExtensionValidationTest : public ValidationTest {
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

    std::string EnabledExtensionsShader(const WGSLExtensionInfo& extension) {
        std::stringstream s;
        for (const char* dependency : extension.dependingExtensionNames) {
            s << "enable " << dependency << ";\n";
        }
        s << "enable " << extension.wgslName << ";\n";
        s << "@compute @workgroup_size(1) fn main() {}";
        return s.str();
    }
};

template <typename T>
class ShaderModuleExtensionValidationTestWithParams
    : public ValidationTestWithParam<T, ShaderModuleExtensionValidationTest> {};

// Test validating WGSL extension on safe device with no required features.
class ShaderModuleExtensionValidationTestSafeNoFeature
    : public ShaderModuleExtensionValidationTest {
  protected:
    bool AllowUnsafeAPIs() override { return false; }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

TEST_F(ShaderModuleExtensionValidationTestSafeNoFeature,
       OnlyStableExtensionsRequiringNoFeatureAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnabledExtensionsShader(extension);

        // On a safe device with no feature required, only stable extensions requiring no features
        // are allowed.
        if (!extension.isExperimental && extension.requiredFeatureNames.size() == 0) {
            utils::CreateShaderModule(device, wgsl.c_str());
        } else {
            ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
        }
    }
}

// Test validating WGSL extension on unsafe device with no required features.
class ShaderModuleExtensionValidationTestUnsafeNoFeature
    : public ShaderModuleExtensionValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

TEST_F(ShaderModuleExtensionValidationTestUnsafeNoFeature,
       OnlyExtensionsRequiringNoFeatureAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnabledExtensionsShader(extension);

        // On an unsafe device with no feature required, only extensions requiring no features are
        // allowed.
        if (extension.requiredFeatureNames.size() == 0) {
            utils::CreateShaderModule(device, wgsl.c_str());
        } else {
            ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
        }
    }
}

// Test validating WGSL extension on safe device with required features set to all.
class ShaderModuleExtensionValidationTestSafeAllFeatures
    : public ShaderModuleExtensionValidationTest {
  protected:
    bool AllowUnsafeAPIs() override { return false; }
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return GetAllFeatures(); }
};

TEST_F(ShaderModuleExtensionValidationTestSafeAllFeatures, OnlyStableExtensionsAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnabledExtensionsShader(extension);

        // On a safe device with all feature required, only stable extensions are allowed.
        if (!extension.isExperimental) {
            utils::CreateShaderModule(device, wgsl.c_str());
        } else {
            ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
        }
    }
}

// Test validating WGSL extension on unsafe device with required features set to all.
class ShaderModuleExtensionValidationTestUnsafeAllFeatures
    : public ShaderModuleExtensionValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return GetAllFeatures(); }
};

TEST_F(ShaderModuleExtensionValidationTestUnsafeAllFeatures, AllExtensionsAllowed) {
    for (auto& extension : kExtensions) {
        std::string wgsl = EnabledExtensionsShader(extension);

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

// Test that each WGSL language extension requires a specific feature to be enabled
class ShaderModuleExtensionValidationTestUnsafeOnlyRequiredFeatures
    : public ShaderModuleExtensionValidationTestWithParams<std::tuple<WGSLExtensionInfo, bool>> {
  public:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        auto [extension, enabled] = GetParam();
        if (enabled) {
            return extension.requiredFeatureNames;
        }
        return {};
    }
};

TEST_P(ShaderModuleExtensionValidationTestUnsafeOnlyRequiredFeatures,
       RequiredExtensionsMustBeEnabled) {
    auto [extension, enabled] = GetParam();
    std::string wgsl = EnabledExtensionsShader(extension);

    if (enabled || extension.requiredFeatureNames.empty()) {
        // On an unsafe device with the required feature enabled, or if it doesn't require one, the
        // wgsl extension should be valid
        utils::CreateShaderModule(device, wgsl.c_str());
    } else {
        // On an unsafe device with the required feature not enabled, the wgsl extension should be
        // invalid
        ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, wgsl.c_str()));
    }
}
INSTANTIATE_TEST_SUITE_P(,
                         ShaderModuleExtensionValidationTestUnsafeOnlyRequiredFeatures,
                         ::testing::Combine(::testing::ValuesIn(kExtensions),
                                            ::testing::Values(true, false)));

class SubgroupSizeControlValidationTest : public ValidationTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ChromiumExperimentalSubgroupSizeControl,
                wgpu::FeatureName::Subgroups};
    }
    void TestTotalInvocationsPerWorkgroupAndSubgroupSize(const std::vector<uint32_t>& workgroupSize,
                                                         uint32_t subgroupSize,
                                                         bool success) {
        for (bool setSubgroupSizeAsOverride : {true, false}) {
            std::ostringstream stream;
            stream << R"(
enable subgroups;
enable chromium_experimental_subgroup_size_control;)";

            if (setSubgroupSizeAsOverride) {
                stream << "override kSubgroupSize : u32;\n";
            } else {
                stream << "const kSubgroupSize = " << subgroupSize << ";\n";
            }

            stream << "@compute @subgroup_size(kSubgroupSize) @workgroup_size(" << workgroupSize[0];
            for (uint32_t i = 1; i < workgroupSize.size(); ++i) {
                stream << ", " << workgroupSize[i];
            }
            stream << ")\n";
            stream << R"(
fn main(@builtin(subgroup_invocation_id) sg_id : u32,
        @builtin(subgroup_size) sg_size : u32) {
    _ = sg_id + sg_size;
})";

            wgpu::ComputePipelineDescriptor pipelineDesc = {};
            pipelineDesc.compute.module = utils::CreateShaderModule(device, stream.str().c_str());

            wgpu::ConstantEntry entry = {};
            if (setSubgroupSizeAsOverride) {
                entry.key = "kSubgroupSize";
                entry.value = static_cast<double>(subgroupSize);
                pipelineDesc.compute.constantCount = 1;
                pipelineDesc.compute.constants = &entry;
            }

            if (success) {
                device.CreateComputePipeline(&pipelineDesc);
            } else {
                ASSERT_DEVICE_ERROR(device.CreateComputePipeline(&pipelineDesc));
            }
        }
    }
};

// Test the X-dimension of the work group size must be a multiple of subgroup size when the
// `@subgroup_size` attribute is used.
TEST_F(SubgroupSizeControlValidationTest, ValidateTotalInvocationsPerWorkgroupAndSubgroupSize) {
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({32}, 16, true);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({16, 4}, 16, true);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({16, 4, 2}, 16, true);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({4, 16}, 16, false);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({4, 2, 16}, 16, false);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({8, 4}, 16, false);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({8, 4, 2}, 32, false);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({24}, 16, false);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({8, 3, 2}, 32, false);
    TestTotalInvocationsPerWorkgroupAndSubgroupSize({32}, 32, true);
}

// Test it is a validation error to use a `@subgroup_size` that is greater than
// `maxExplicitComputeSubgroupSize` or less than `minExplicitComputeSubgroupSize` on current
// adapter.
TEST_F(SubgroupSizeControlValidationTest, ValidateExplicitComputeSubgroupSizes) {
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesExplicitComputeSubgroupSizeConfigs subgroupSizeConfigs;
    info.nextInChain = &subgroupSizeConfigs;
    adapter.GetInfo(&info);

    for (uint32_t subgroupSize = subgroupSizeConfigs.minExplicitComputeSubgroupSize / 2;
         subgroupSize <= subgroupSizeConfigs.maxExplicitComputeSubgroupSize * 2;
         subgroupSize *= 2) {
        ASSERT_TRUE(IsPowerOfTwo(subgroupSize));
        bool success = subgroupSize >= subgroupSizeConfigs.minExplicitComputeSubgroupSize &&
                       subgroupSize <= subgroupSizeConfigs.maxExplicitComputeSubgroupSize;
        TestTotalInvocationsPerWorkgroupAndSubgroupSize({subgroupSize}, subgroupSize, success);
    }
}

// Test it is a validation error to use a `@subgroup_size` that makes the total invocations per
// workgroup exceed the product of `@subgroup_size` and `maxComputeWorkgroupSubgroups` on current
// adapter.
TEST_F(SubgroupSizeControlValidationTest, ValidateMaxComputeWorkgroupSubgroups) {
    wgpu::AdapterInfo info;
    wgpu::AdapterPropertiesExplicitComputeSubgroupSizeConfigs subgroupSizeConfigs;
    info.nextInChain = &subgroupSizeConfigs;
    adapter.GetInfo(&info);
    wgpu::Limits limits;
    adapter.GetLimits(&limits);

    uint32_t maxWorkgroupSubgroups = subgroupSizeConfigs.maxComputeWorkgroupSubgroups;
    uint32_t maxInvocationsPerWorkgroup = limits.maxComputeInvocationsPerWorkgroup;

    for (uint32_t subgroupSize = subgroupSizeConfigs.minExplicitComputeSubgroupSize;
         subgroupSize <= subgroupSizeConfigs.maxExplicitComputeSubgroupSize; subgroupSize *= 2) {
        ASSERT_TRUE(IsPowerOfTwo(subgroupSize));
        uint32_t totalInvocations = maxInvocationsPerWorkgroup;
        uint32_t workgroupSizeX = subgroupSize;
        uint32_t workgroupSizeY = totalInvocations / workgroupSizeX;
        ASSERT_LE(workgroupSizeY, limits.maxComputeWorkgroupSizeY);
        bool success = maxInvocationsPerWorkgroup <= subgroupSize * maxWorkgroupSubgroups;
        TestTotalInvocationsPerWorkgroupAndSubgroupSize({workgroupSizeX, workgroupSizeY},
                                                        subgroupSize, success);
    }
}

}  // anonymous namespace
}  // namespace dawn
