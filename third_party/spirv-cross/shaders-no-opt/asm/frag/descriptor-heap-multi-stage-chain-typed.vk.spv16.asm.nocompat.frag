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
			   OpDecorate %readonly NonWritable
			   OpDecorate %writeonly NonReadable
               OpMemberDecorate %SSBO 0 Offset 0
               OpDecorateId %_runtimearr_20 ArrayStrideIdEXT %21
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_float = OpTypePointer Output %float
  %FragColor = OpVariable %_ptr_Output_float Output
    %float_0 = OpConstant %float 0
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
       %SSBO = OpTypeStruct %_runtimearr_v4float
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
%_ptr_StorageBuffer_SSBO = OpTypePointer StorageBuffer %SSBO
%_ptr_StorageBuffer_v4float = OpTypePointer StorageBuffer %v4float
%_ptr_StorageBuffer_float = OpTypePointer StorageBuffer %float
         %20 = OpTypeBufferEXT StorageBuffer
         %21 = OpConstantSizeOfEXT %int %20
%_runtimearr_20 = OpTypeRuntimeArray %20
       %uint = OpTypeInt 32 0
       %main = OpFunction %void None %3
          %5 = OpLabel
         %19 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_20 %resource_heap %int_2
         %readonly = OpBufferPointerEXT %_ptr_StorageBuffer_SSBO %19
         %writeonly = OpBufferPointerEXT %_ptr_StorageBuffer_SSBO %19

		 %chain_load = OpAccessChain %_ptr_StorageBuffer_v4float %readonly %int_0 %int_2
		 %chain_load_again = OpAccessChain %_ptr_StorageBuffer_float %chain_load %int_2

		 %chain_store = OpAccessChain %_ptr_StorageBuffer_v4float %writeonly %int_0 %int_0
		 %chain_store_again = OpAccessChain %_ptr_StorageBuffer_float %chain_store %int_2

		 %loaded = OpLoad %float %chain_load_again
		 OpStore %chain_store_again %loaded
               OpStore %FragColor %loaded
               OpReturn
               OpFunctionEnd
