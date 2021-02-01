// Copyright (c) 2018 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Validation tests for WebGPU env specific checks

#include <string>

#include "gmock/gmock.h"
#include "test/val/val_fixtures.h"

namespace spvtools {
namespace val {
namespace {

using testing::HasSubstr;

using ValidateWebGPU = spvtest::ValidateBase<bool>;

TEST_F(ValidateWebGPU, OpUndefIsDisallowed) {
  std::string spirv = R"(
          OpCapability Shader
          OpCapability VulkanMemoryModelKHR
          OpExtension "SPV_KHR_vulkan_memory_model"
          OpMemoryModel Logical VulkanKHR
          OpEntryPoint Vertex %func "shader"
%float  = OpTypeFloat 32
%1      = OpUndef %float
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%func   = OpFunction %void None %void_f
%label  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  // Control case: OpUndef is allowed in SPIR-V 1.3
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_UNIVERSAL_1_3));

  // Control case: OpUndef is disallowed in the WebGPU env
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(), HasSubstr("OpUndef is disallowed"));
}

TEST_F(ValidateWebGPU, OpNameIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
            OpName %1 "foo"
       %1 = OpTypeFloat 32
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpMemberNameIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
            OpMemberName %2 0 "foo"
       %1 = OpTypeFloat 32
       %2 = OpTypeStruct %1
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd

)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpSourceIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
            OpSource GLSL 450
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpSourceContinuedIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
            OpSource GLSL 450
            OpSourceContinued "I am a happy shader! Yay! ;"
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpSourceExtensionIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
            OpSourceExtension "bar"
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpStringIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
       %1 = OpString "foo"
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpLineIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
       %1 = OpString "minimal.vert"
            OpLine %1 1 1
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, OpNoLineIsAllowed) {
  std::string spirv = R"(
            OpCapability Shader
            OpCapability VulkanMemoryModelKHR
            OpExtension "SPV_KHR_vulkan_memory_model"
            OpMemoryModel Logical VulkanKHR
            OpEntryPoint Vertex %func "shader"
            OpNoLine
  %void   = OpTypeVoid
  %void_f = OpTypeFunction %void
  %func   = OpFunction %void None %void_f
  %label  = OpLabel
            OpReturn
            OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, LogicalAddressingVulkanKHRMemoryGood) {
  std::string spirv = R"(
          OpCapability Shader
          OpCapability VulkanMemoryModelKHR
          OpExtension "SPV_KHR_vulkan_memory_model"
          OpMemoryModel Logical VulkanKHR
          OpEntryPoint Vertex %func "shader"
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%func   = OpFunction %void None %void_f
%label  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, LogicalAddressingGLSL450MemoryGood) {
  std::string spirv = R"(
          OpCapability Shader
          OpMemoryModel Logical GLSL450
          OpEntryPoint Vertex %func "shader"
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%func   = OpFunction %void None %void_f
%label  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, LogicalAddressingSimpleMemoryGood) {
  std::string spirv = R"(
          OpCapability Shader
          OpMemoryModel Logical Simple
          OpEntryPoint Vertex %func "shader"
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%func   = OpFunction %void None %void_f
%label  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(spirv);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, KernelIsBad) {
  std::string spirv = R"(
     OpCapability Kernel
     OpMemoryModel Logical Simple
     OpNoLine
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_CAPABILITY,
            ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Capability Kernel is not allowed by WebGPU "
                        "specification (or requires extension)"));
}

TEST_F(ValidateWebGPU, OpenCLMemoryModelBad) {
  std::string spirv = R"(
     OpCapability Shader
     OpMemoryModel Logical OpenCL
     OpNoLine
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_CAPABILITY,
            ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("Operand 2 of MemoryModel requires one of these "
                        "capabilities: Kernel"));
}

TEST_F(ValidateWebGPU, AllowListedExtendedInstructionsImportGood) {
  std::string spirv = R"(
          OpCapability Shader
          OpCapability VulkanMemoryModelKHR
          OpExtension "SPV_KHR_vulkan_memory_model"
%1      = OpExtInstImport "GLSL.std.450"
          OpMemoryModel Logical VulkanKHR
          OpEntryPoint Vertex %func "shader"
%void   = OpTypeVoid
%void_f = OpTypeFunction %void
%func   = OpFunction %void None %void_f
%label  = OpLabel
          OpReturn
          OpFunctionEnd
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, NonAllowListedExtendedInstructionsImportBad) {
  std::string spirv = R"(
     OpCapability Shader
     OpCapability VulkanMemoryModelKHR
     OpExtension "SPV_KHR_vulkan_memory_model"
%1 = OpExtInstImport "OpenCL.std"
     OpMemoryModel Logical VulkanKHR
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For WebGPU, the only valid parameter to "
                        "OpExtInstImport is \"GLSL.std.450\".\n  %1 = "
                        "OpExtInstImport \"OpenCL.std\"\n"));
}

TEST_F(ValidateWebGPU, NonVulkanKHRMemoryModelExtensionBad) {
  std::string spirv = R"(
     OpCapability Shader
     OpCapability VulkanMemoryModelKHR
     OpExtension "SPV_KHR_8bit_storage"
     OpExtension "SPV_KHR_vulkan_memory_model"
     OpMemoryModel Logical VulkanKHR
)";

  CompileSuccessfully(spirv);

  EXPECT_EQ(SPV_ERROR_INVALID_DATA, ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("For WebGPU, the only valid parameter to OpExtension "
                        "is \"SPV_KHR_vulkan_memory_model\".\n  OpExtension "
                        "\"SPV_KHR_8bit_storage\"\n"));
}

spv_binary GenerateTrivialBinary(bool need_little_endian) {
  // Smallest possible valid WebGPU SPIR-V binary in little endian. Contains all
  // the required boilerplate and a trivial entry point function.
  static const uint8_t binary_bytes[] = {
      // clang-format off
    0x03, 0x02, 0x23, 0x07, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00, 0x07, 0x00,
    0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00, 0xE1, 0x14, 0x00, 0x00,
    0x0A, 0x00, 0x08, 0x00, 0x53, 0x50, 0x56, 0x5F, 0x4B, 0x48, 0x52, 0x5F,
    0x76, 0x75, 0x6C, 0x6B, 0x61, 0x6E, 0x5F, 0x6D, 0x65, 0x6D, 0x6F, 0x72,
    0x79, 0x5F, 0x6D, 0x6F, 0x64, 0x65, 0x6C, 0x00, 0x0E, 0x00, 0x03, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x05, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x73, 0x68, 0x61, 0x64,
    0x65, 0x72, 0x00, 0x00, 0x13, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x21, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x02, 0x00,
    0x04, 0x00, 0x00, 0x00, 0xFD, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00
      // clang-format on
  };
  static const size_t word_count = sizeof(binary_bytes) / sizeof(uint32_t);
  std::unique_ptr<spv_binary_t> result(new spv_binary_t);
  if (!result) return nullptr;

  result->wordCount = word_count;
  result->code = new uint32_t[word_count];
  if (!result->code) return nullptr;

  if (need_little_endian) {
    memcpy(result->code, binary_bytes, sizeof(binary_bytes));
  } else {
    uint8_t* code_bytes = reinterpret_cast<uint8_t*>(result->code);
    for (size_t word = 0; word < word_count; ++word) {
      code_bytes[4 * word] = binary_bytes[4 * word + 3];
      code_bytes[4 * word + 1] = binary_bytes[4 * word + 2];
      code_bytes[4 * word + 2] = binary_bytes[4 * word + 1];
      code_bytes[4 * word + 3] = binary_bytes[4 * word];
    }
  }

  return result.release();
}

TEST_F(ValidateWebGPU, LittleEndianGood) {
  DestroyBinary();
  binary_ = GenerateTrivialBinary(true);
  EXPECT_EQ(SPV_SUCCESS, ValidateInstructions(SPV_ENV_WEBGPU_0));
}

TEST_F(ValidateWebGPU, BigEndianBad) {
  DestroyBinary();
  binary_ = GenerateTrivialBinary(false);
  EXPECT_EQ(SPV_ERROR_INVALID_BINARY, ValidateInstructions(SPV_ENV_WEBGPU_0));
  EXPECT_THAT(getDiagnosticString(),
              HasSubstr("WebGPU requires SPIR-V to be little endian."));
}

}  // namespace
}  // namespace val
}  // namespace spvtools
