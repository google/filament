; SPIR-V
; Version: 1.3
; Generator: Google spiregg; 0
; Bound: 81
; Schema: 0
               OpCapability Shader
               OpCapability Sampled1D
               OpCapability Image1D
               OpCapability SampledBuffer
               OpCapability ImageBuffer
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %PsTextureLoadArray "main" %gl_FragCoord %out_var_SV_TARGET
               OpExecutionMode %PsTextureLoadArray OriginUpperLeft
               OpSource HLSL 500
               OpName %type_2d_image "type.2d.image"
               OpName %type_gCBuffarrayIndex "type.gCBuffarrayIndex"
               OpMemberName %type_gCBuffarrayIndex 0 "gArrayIndex"
               OpName %gCBuffarrayIndex "gCBuffarrayIndex"
               OpName %g_textureArray0 "g_textureArray0"
               OpName %g_textureArray1 "g_textureArray1"
               OpName %g_textureArray2 "g_textureArray2"
               OpName %g_textureArray3 "g_textureArray3"
               OpName %out_var_SV_TARGET "out.var.SV_TARGET"
               OpName %PsTextureLoadArray "PsTextureLoadArray"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorate %out_var_SV_TARGET Location 0
               OpDecorate %gCBuffarrayIndex DescriptorSet 0
               OpDecorate %gCBuffarrayIndex Binding 0
               OpDecorate %g_textureArray0 DescriptorSet 0
               OpDecorate %g_textureArray0 Binding 0
               OpDecorate %g_textureArray1 DescriptorSet 0
               OpDecorate %g_textureArray1 Binding 1
               OpDecorate %g_textureArray2 DescriptorSet 0
               OpDecorate %g_textureArray2 Binding 2
               OpDecorate %g_textureArray3 DescriptorSet 0
               OpDecorate %g_textureArray3 Binding 3
               OpMemberDecorate %type_gCBuffarrayIndex 0 Offset 0
               OpDecorate %type_gCBuffarrayIndex Block
       %uint = OpTypeInt 32 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %float = OpTypeFloat 32
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
    %v4float = OpTypeVector %float 4
         %18 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_gCBuffarrayIndex = OpTypeStruct %uint
%_ptr_Uniform_type_gCBuffarrayIndex = OpTypePointer Uniform %type_gCBuffarrayIndex
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
      %v3int = OpTypeVector %int 3
      %v2int = OpTypeVector %int 2
%gCBuffarrayIndex = OpVariable %_ptr_Uniform_type_gCBuffarrayIndex Uniform
%g_textureArray0 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%g_textureArray1 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%g_textureArray2 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%g_textureArray3 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%out_var_SV_TARGET = OpVariable %_ptr_Output_v4float Output
     %uint_0 = OpConstant %uint 0
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
       %true = OpConstantTrue %bool
         %32 = OpUndef %v4float
%PsTextureLoadArray = OpFunction %void None %24
         %33 = OpLabel
         %34 = OpLoad %v4float %gl_FragCoord
               OpSelectionMerge %35 None
               OpSwitch %uint_0 %36
         %36 = OpLabel
         %37 = OpAccessChain %_ptr_Uniform_uint %gCBuffarrayIndex %int_0
         %38 = OpLoad %uint %37
               OpSelectionMerge %39 None
               OpSwitch %38 %40 0 %41 1 %42 2 %43 3 %44
         %41 = OpLabel
         %45 = OpCompositeExtract %float %34 0
         %46 = OpCompositeExtract %float %34 1
         %47 = OpConvertFToS %int %45
         %48 = OpConvertFToS %int %46
         %49 = OpCompositeConstruct %v3int %47 %48 %int_0
         %50 = OpVectorShuffle %v2int %49 %49 0 1
         %51 = OpLoad %type_2d_image %g_textureArray0
         %52 = OpImageFetch %v4float %51 %50 Lod %int_0
               OpBranch %39
         %42 = OpLabel
         %53 = OpCompositeExtract %float %34 0
         %54 = OpCompositeExtract %float %34 1
         %55 = OpConvertFToS %int %53
         %56 = OpConvertFToS %int %54
         %57 = OpCompositeConstruct %v3int %55 %56 %int_0
         %58 = OpVectorShuffle %v2int %57 %57 0 1
         %59 = OpLoad %type_2d_image %g_textureArray1
         %60 = OpImageFetch %v4float %59 %58 Lod %int_0
               OpBranch %39
         %43 = OpLabel
         %61 = OpCompositeExtract %float %34 0
         %62 = OpCompositeExtract %float %34 1
         %63 = OpConvertFToS %int %61
         %64 = OpConvertFToS %int %62
         %65 = OpCompositeConstruct %v3int %63 %64 %int_0
         %66 = OpVectorShuffle %v2int %65 %65 0 1
         %67 = OpLoad %type_2d_image %g_textureArray2
         %68 = OpImageFetch %v4float %67 %66 Lod %int_0
               OpBranch %39
         %44 = OpLabel
         %69 = OpCompositeExtract %float %34 0
         %70 = OpCompositeExtract %float %34 1
         %71 = OpConvertFToS %int %69
         %72 = OpConvertFToS %int %70
         %73 = OpCompositeConstruct %v3int %71 %72 %int_0
         %74 = OpVectorShuffle %v2int %73 %73 0 1
         %75 = OpLoad %type_2d_image %g_textureArray3
         %76 = OpImageFetch %v4float %75 %74 Lod %int_0
               OpBranch %39
         %40 = OpLabel
               OpBranch %39
         %39 = OpLabel
         %77 = OpPhi %v4float %52 %41 %60 %42 %68 %43 %76 %44 %32 %40
         %78 = OpPhi %bool %true %41 %true %42 %true %43 %true %44 %false %40
               OpSelectionMerge %79 None
               OpBranchConditional %78 %35 %79
         %79 = OpLabel
               OpBranch %35
         %35 = OpLabel
         %80 = OpPhi %v4float %77 %39 %18 %79
               OpStore %out_var_SV_TARGET %80
               OpReturn
               OpFunctionEnd
