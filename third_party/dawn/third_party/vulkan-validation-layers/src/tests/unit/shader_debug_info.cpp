/*
 * Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/shader_helper.h"

class NegativeShaderDebugInfo : public VkLayerTest {};

TEST_F(NegativeShaderDebugInfo, FloatShaderDebugInfo) {
    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredExtensions(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());

    std::string cs_source = R"(
               OpCapability Shader
               OpExtension "SPV_KHR_non_semantic_info"
          %1 = OpExtInstImport "NonSemantic.Shader.DebugInfo.100"
          %3 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint GLCompute %main "main"
               OpExecutionMode %main LocalSize 1 1 1
          %2 = OpString "a.comp"
          %8 = OpString "uint"
         %16 = OpString "main"
         %19 = OpString "#version 450
#extension GL_EXT_shader_atomic_float : enable
#extension GL_KHR_memory_scope_semantics : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float32 : enable

layout(set = 0, binding = 0) buffer ssbo { float32_t y; };
void main() {
    y = 1 + atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);
}"
         %29 = OpString "float"
         %33 = OpString "y"
         %36 = OpString "ssbo"
         %43 = OpString ""
         %45 = OpString "int"
               OpSourceExtension "GL_EXT_shader_atomic_float"
               OpSourceExtension "GL_EXT_shader_explicit_arithmetic_types_float32"
               OpSourceExtension "GL_KHR_memory_scope_semantics"
               OpName %main "main"
               OpName %ssbo "ssbo"
               OpMemberName %ssbo 0 "y"
               OpName %_ ""
               OpModuleProcessed "client vulkan100"
               OpModuleProcessed "target-env spirv1.3"
               OpModuleProcessed "target-env vulkan1.1"
               OpModuleProcessed "entry-point main"
               OpDecorate %ssbo Block
               OpMemberDecorate %ssbo 0 Offset 0
               OpDecorate %_ Binding 0
               OpDecorate %_ DescriptorSet 0
       %void = OpTypeVoid
          %5 = OpTypeFunction %void
       %uint = OpTypeInt 32 0
    %uint_32 = OpConstant %uint 32
     %uint_6 = OpConstant %uint 6
     %uint_0 = OpConstant %uint 0
          %9 = OpExtInst %void %1 DebugTypeBasic %8 %uint_32 %uint_6 %uint_0
     %uint_3 = OpConstant %uint 3
          %6 = OpExtInst %void %1 DebugTypeFunction %uint_3 %void
         %18 = OpExtInst %void %1 DebugSource %2 %19
     %uint_7 = OpConstant %uint 7
     %uint_1 = OpConstant %uint 1
     %uint_4 = OpConstant %uint 4
     %uint_2 = OpConstant %uint 2
         %21 = OpExtInst %void %1 DebugCompilationUnit %uint_1 %uint_4 %18 %uint_2
         %17 = OpExtInst %void %1 DebugFunction %16 %6 %18 %uint_7 %uint_0 %21 %16 %uint_3 %uint_7
      %float = OpTypeFloat 32
         %30 = OpExtInst %void %1 DebugTypeBasic %29 %uint_32 %uint_3 %uint_0
       %ssbo = OpTypeStruct %float
    %uint_54 = OpConstant %uint 54
         %32 = OpExtInst %void %1 DebugTypeMember %33 %30 %18 %uint_6 %uint_54 %uint_0 %uint_0 %uint_3
     %uint_8 = OpConstant %uint 8
         %35 = OpExtInst %void %1 DebugTypeComposite %36 %uint_1 %18 %uint_8 %uint_0 %21 %36 %uint_0 %uint_3 %32
%_ptr_StorageBuffer_ssbo = OpTypePointer StorageBuffer %ssbo
    %uint_12 = OpConstant %uint 12
         %40 = OpExtInst %void %1 DebugTypePointer %35 %uint_12 %uint_0
          %_ = OpVariable %_ptr_StorageBuffer_ssbo StorageBuffer
         %42 = OpExtInst %void %1 DebugGlobalVariable %43 %35 %18 %uint_8 %uint_0 %21 %43 %_ %uint_8
        %int = OpTypeInt 32 1
         %46 = OpExtInst %void %1 DebugTypeBasic %45 %uint_32 %uint_4 %uint_0
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
         %50 = OpExtInst %void %1 DebugTypePointer %30 %uint_12 %uint_0
      %int_1 = OpConstant %int 1
     %int_64 = OpConstant %int 64
    %uint_64 = OpConstant %uint 64
     %uint_9 = OpConstant %uint 9
       %main = OpFunction %void None %5
         %15 = OpLabel
         %26 = OpExtInst %void %1 DebugScope %17
         %27 = OpExtInst %void %1 DebugLine %18 %uint_7 %uint_7 %uint_0 %uint_0
         %25 = OpExtInst %void %1 DebugFunctionDefinition %17 %main
         %52 = OpExtInst %void %1 DebugLine %18 %uint_8 %uint_8 %uint_0 %uint_0
         %51 = OpAccessChain %_ptr_StorageBuffer_float %_ %int_0
         %56 = OpAtomicLoad %float %51 %int_1 %uint_64
         %57 = OpFAdd %float %float_1 %56
         %58 = OpAccessChain %_ptr_StorageBuffer_float %_ %int_0
               OpStore %58 %57
         %59 = OpExtInst %void %1 DebugLine %18 %uint_9 %uint_9 %uint_0 %uint_0
               OpReturn
               OpFunctionEnd
    )";

    // VUID-RuntimeSpirv-None-06284
    m_errorMonitor->SetDesiredError("atomicLoad(y, gl_ScopeDevice, gl_StorageSemanticsBuffer, gl_SemanticsRelaxed);");
    VkShaderObj const cs(this, cs_source.c_str(), VK_SHADER_STAGE_COMPUTE_BIT, SPV_ENV_VULKAN_1_1, SPV_SOURCE_ASM);
    m_errorMonitor->VerifyFound();
}
