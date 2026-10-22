               OpCapability Shader
               OpCapability Sampled1D
               OpCapability RayQueryKHR
               OpCapability UntypedPointersKHR
               OpCapability DescriptorHeapEXT
               OpExtension "SPV_EXT_descriptor_heap"
               OpExtension "SPV_KHR_ray_query"
               OpExtension "SPV_KHR_untyped_pointers"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %FragColor %gl_FragCoord %resource_heap %rq
               OpExecutionMode %main OriginUpperLeft
               OpSource GLSL 460
               OpSourceExtension "GL_EXT_descriptor_heap"
               OpSourceExtension "GL_EXT_nonuniform_qualifier"
               OpSourceExtension "GL_EXT_ray_query"
               OpSourceExtension "GL_EXT_samplerless_texture_functions"
               OpSourceExtension "GL_EXT_scalar_block_layout"
               OpSourceExtension "GL_EXT_shader_image_load_formatted"
               OpName %main "main"
               OpName %FragColor "FragColor"
               OpName %resource_heap "resource_heap"
               OpName %gl_FragCoord "gl_FragCoord"
               OpName %UBO140 "UBO140"
               OpMemberName %UBO140 0 "data"
               OpName %rq "rq"
               OpDecorate %FragColor Location 0
               OpDecorate %resource_heap BuiltIn ResourceHeapEXT
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %_arr_float_uint_2 ArrayStride 16
               OpDecorate %UBO140 Block
               OpDecorateId %_runtimearr_23 ArrayStrideIdEXT %heap_size
               OpDecorateId %_runtimearr_39 ArrayStrideIdEXT %heap_size
               OpDecorateId %_runtimearr_51 ArrayStrideIdEXT %heap_size
               OpMemberDecorate %UBO140 0 Offset 0
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
%_ptr_Input_v4float = OpTypePointer Input %v4float
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
       %uint = OpTypeInt 32 0
     %uint_0 = OpConstant %uint 0
%_ptr_Input_float = OpTypePointer Input %float
        %int = OpTypeInt 32 1
         %23 = OpTypeImage %float 1D 0 0 0 1 Unknown
%_ptr_Uniform = OpTypeUntypedPointerKHR Uniform
      %int_0 = OpConstant %int 0
     %int_50 = OpConstant %int 50
     %uint_2 = OpConstant %uint 2
%_arr_float_uint_2 = OpTypeArray %float %uint_2
     %UBO140 = OpTypeStruct %_arr_float_uint_2
      %int_1 = OpConstant %int 1
	  %bool = OpTypeBool

         %39 = OpTypeBufferEXT Uniform
         %51 = OpTypeAccelerationStructureKHR
         %buffer_size = OpConstantSizeOfEXT %int %39
         %image_size = OpConstantSizeOfEXT %int %23

		 ; D3D12 style sizing
		 %image_is_greater = OpSpecConstantOp %bool UGreaterThan %image_size %buffer_size
		 %heap_size = OpSpecConstantOp %int Select %image_is_greater %image_size %buffer_size

%_runtimearr_23 = OpTypeRuntimeArray %23
%_runtimearr_39 = OpTypeRuntimeArray %39
%_runtimearr_51 = OpTypeRuntimeArray %51

         %48 = OpTypeRayQueryKHR
%_ptr_Private_48 = OpTypePointer Private %48
         %rq = OpVariable %_ptr_Private_48 Private
    %v3float = OpTypeVector %float 3
         %57 = OpConstantComposite %v3float %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
         %59 = OpConstantComposite %v3float %float_1 %float_0 %float_0
       %main = OpFunction %void None %3
          %5 = OpLabel
               OpStore %FragColor %11
         %19 = OpAccessChain %_ptr_Input_float %gl_FragCoord %uint_0
         %20 = OpLoad %float %19
         %22 = OpConvertFToS %int %20
         %25 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_23 %resource_heap %22
         %28 = OpLoad %23 %25
         %30 = OpImageFetch %v4float %28 %int_0 Lod %int_0
         %31 = OpLoad %v4float %FragColor
         %32 = OpFAdd %v4float %31 %30
               OpStore %FragColor %32
         %38 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_39 %resource_heap %int_50
         %42 = OpBufferPointerEXT %_ptr_Uniform %38
         %43 = OpUntypedAccessChainKHR %_ptr_Uniform %UBO140 %42 %int_0 %int_1
         %44 = OpLoad %float %43
         %45 = OpLoad %v4float %FragColor
         %46 = OpCompositeConstruct %v4float %44 %44 %44 %44
         %47 = OpFAdd %v4float %45 %46
               OpStore %FragColor %47
         %52 = OpUntypedAccessChainKHR %_ptr_UniformConstant %_runtimearr_51 %resource_heap %int_50
         %55 = OpLoad %51 %52
               OpRayQueryInitializeKHR %rq %55 %uint_0 %uint_0 %57 %float_0 %59 %float_1
               OpReturn
               OpFunctionEnd
