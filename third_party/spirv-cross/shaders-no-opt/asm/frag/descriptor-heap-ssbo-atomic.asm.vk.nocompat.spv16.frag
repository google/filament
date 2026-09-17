; SPIR-V
; Version: 1.6
; Generator: Khronos Glslang Reference Front End; 11
; Bound: 32
; Schema: 0
               OpCapability Shader
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %resource_heap %gl_FragCoord
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpSourceExtension "GL_EXT_ray_query"
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
               OpSourceExtension "GL_EXT_scalar_block_layout"
               OpSourceExtension "GL_EXT_shader_image_load_formatted"
               OpName %main "main"
               OpName %desc_index "desc_index"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %resource_heap "resource_heap"
               OpName %SSBOAtomic "SSBOAtomic"
               OpMemberName %SSBOAtomic 0 "data"
               OpMemberName %SSBOAtomic 1 "data2"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %SSBOAtomic Block
               OpMemberDecorate %SSBOAtomic 0 Offset 0
               OpMemberDecorate %SSBOAtomic 1 Offset 4
               OpDecorateId %_runtimearr_26 ArrayStrideIdEXT %27
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
%_ptr_UniformConstant = OpTypeUntypedPointerKHR UniformConstant
%resource_heap = OpUntypedVariableKHR %_ptr_UniformConstant UniformConstant
 %SSBOAtomic = OpTypeStruct %uint %uint
      %int_1 = OpConstant %int 1
%_ptr_StorageBuffer = OpTypeUntypedPointerKHR StorageBuffer
%_ptr_StorageBuffer_SSBOAtomic = OpTypePointer StorageBuffer %SSBOAtomic
%_ptr_StorageBuffer_uint = OpTypePointer StorageBuffer %uint
         %26 = OpTypeBufferEXT StorageBuffer
         %27 = OpConstantSizeOfEXT %int %26
%_runtimearr_26 = OpTypeRuntimeArray %26
     %uint_1 = OpConstant %uint 1
       %main = OpFunction %void None %3
          %5 = OpLabel
 %desc_index = OpVariable %_ptr_Function_int Function
         %16 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %17 = OpLoad %float %16
         %18 = OpConvertFToS %int %17
               OpStore %desc_index %18
         %21 = OpLoad %int %desc_index
         %25 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_26 %resource_heap %21

         %untyped_ptr = OpBufferPointerEXT %_ptr_StorageBuffer %25
		 %untyped_chain = OpUntypedAccessChainKHR %_ptr_StorageBuffer %SSBOAtomic %untyped_ptr %uint_1
         %dummy0 = OpAtomicIAdd %uint %untyped_chain %uint_1 %uint_0 %uint_1

		%typed_ptr = OpBufferPointerEXT %_ptr_StorageBuffer_SSBOAtomic %25
		 %typed_chain = OpAccessChain %_ptr_StorageBuffer_uint %typed_ptr %uint_1
         %dummy1 = OpAtomicIAdd %uint %typed_chain %uint_1 %uint_0 %uint_1

               OpReturn
               OpFunctionEnd
