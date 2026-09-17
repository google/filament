; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 31
; Schema: 0
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %resource_heap
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %resource_heap "resource_heap"
               OpName %SSBO "SSBO"
               OpMemberName %SSBO 0 "data"
               OpDecorate %FragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpDecorate %SSBO Block
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %21
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
  %FragColor = OpVariable %_ptr_Output_v4float Output
    %float_0 = OpConstant %float 0
         %11 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
       %SSBO = OpTypeStruct %_runtimearr_v4float
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
         %20 = OpTypeBufferEXT StorageBuffer
         %21 = OpConstantSizeOfEXT %int %20
%_runtimearr_20 = OpTypeRuntimeArray %20
       %uint = OpTypeInt 32 0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %FragColor %11
         %19 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_20 %resource_heap %int_2
         %23 = OpBufferPointerEXT %_ptr_StorageBuffer %19
         %25 = OpUntypedArrayLengthKHR %uint %SSBO %23 0
         %26 = OpBitcast %int %25
         %27 = OpConvertSToF %float %26
         %28 = OpLoad %v4float %FragColor
         %29 = OpCompositeConstruct %v4float %27 %27 %27 %27
         %30 = OpFAdd %v4float %28 %29
               OpStore %FragColor %30
               OpReturn
               OpFunctionEnd
