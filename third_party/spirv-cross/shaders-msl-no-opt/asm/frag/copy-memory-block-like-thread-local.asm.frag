; SPIR-V
; Version: 1.3
; Generator: Google rspirv; 0
; Bound: 43
; Schema: 0
               OpCapability ImageQuery
               OpCapability Int8
               OpCapability RuntimeDescriptorArray
               OpCapability StorageImageWriteWithoutFormat
               OpCapability Shader
               OpCapability VulkanMemoryModel
               OpExtension "SPV_EXT_descriptor_indexing"
               OpExtension "SPV_KHR_vulkan_memory_model"
               OpMemoryModel Logical Vulkan
               OpEntryPoint Fragment %1 "main"
               OpExecutionMode %1 OriginUpperLeft
               OpDecorate %2 ArrayStride 4
               OpMemberDecorate %3 0 Offset 0
          %4 = OpTypeInt 32 0
          %5 = OpTypeFloat 32
          %6 = OpTypePointer Function %5
          %7 = OpTypeVoid
          %8 = OpTypeFunction %7
          %9 = OpConstant %4 0
         %10 = OpConstant %4 1
         %11 = OpConstant %4 2
         %12 = OpConstant %4 4
         %13 = OpConstant %4 3
         %14 = OpConstant %5 0
          %2 = OpTypeArray %5 %12
         %15 = OpTypePointer Function %2
         %16 = OpTypeFunction %7 %15
          %3 = OpTypeStruct %2
         %17 = OpTypePointer Function %3
          %1 = OpFunction %7 None %8
         %31 = OpLabel
         %33 = OpVariable %17 Function
         %34 = OpVariable %15 Function
         %39 = OpAccessChain %6 %34 %9
               OpStore %39 %14
         %40 = OpAccessChain %6 %34 %10
               OpStore %40 %14
         %41 = OpAccessChain %6 %34 %11
               OpStore %41 %14
         %42 = OpAccessChain %6 %34 %13
               OpStore %42 %14
         %37 = OpAccessChain %15 %33 %9
               OpCopyMemory %37 %34
               OpReturn
               OpFunctionEnd
