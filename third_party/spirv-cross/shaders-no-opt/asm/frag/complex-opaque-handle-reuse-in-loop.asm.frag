; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 71
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %ps_main "main" %out_var_SV_TARGET1
               OpExecutionMode %ps_main OriginUpperLeft
               OpSource HLSL 600
               OpName %type_scene "type.scene"
               OpMemberName %type_scene 0 "myConsts"
               OpName %MyConsts "MyConsts"
               OpMemberName %MyConsts 0 "opt"
               OpName %scene "scene"
               OpName %type_sampler "type.sampler"
               OpName %mySampler "mySampler"
               OpName %type_2d_image "type.2d.image"
               OpName %texTable "texTable"
               OpName %out_var_SV_TARGET1 "out.var.SV_TARGET1"
               OpName %ps_main "ps_main"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %out_var_SV_TARGET1 Location 1
               OpDecorate %scene DescriptorSet 0
               OpDecorate %scene Binding 3
               OpDecorate %mySampler DescriptorSet 0
               OpDecorate %mySampler Binding 2
               OpDecorate %texTable DescriptorSet 0
               OpDecorate %texTable Binding 0
               OpMemberDecorate %MyConsts 0 Offset 0
               OpMemberDecorate %type_scene 0 Offset 0
               OpDecorate %type_scene Block
      %float = OpTypeFloat 32
    %float_1 = OpConstant %float 1
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
       %uint = OpTypeInt 32 0
%uint_16777215 = OpConstant %uint 16777215
     %uint_0 = OpConstant %uint 0
    %float_0 = OpConstant %float 0
         %21 = OpConstantComposite %v2float %float_0 %float_0
   %MyConsts = OpTypeStruct %uint
 %type_scene = OpTypeStruct %MyConsts
%_ptr_Uniform_type_scene = OpTypePointer Uniform %type_scene
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
     %uint_1 = OpConstant %uint 1
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_arr_type_2d_image_uint_1 = OpTypeArray %type_2d_image %uint_1
%_ptr_UniformConstant__arr_type_2d_image_uint_1 = OpTypePointer UniformConstant %_arr_type_2d_image_uint_1
%_ptr_Output_uint = OpTypePointer Output %uint
       %void = OpTypeVoid
         %29 = OpTypeFunction %void
     %v4uint = OpTypeVector %uint 4
    %v3float = OpTypeVector %float 3
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
       %bool = OpTypeBool
%type_sampled_image = OpTypeSampledImage %type_2d_image
    %v4float = OpTypeVector %float 4
      %scene = OpVariable %_ptr_Uniform_type_scene Uniform
  %mySampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %texTable = OpVariable %_ptr_UniformConstant__arr_type_2d_image_uint_1 UniformConstant
%out_var_SV_TARGET1 = OpVariable %_ptr_Output_uint Output
   %float_n1 = OpConstant %float -1
         %37 = OpUndef %v4uint
    %ps_main = OpFunction %void None %29
         %38 = OpLabel
               OpSelectionMerge %39 None
               OpSwitch %uint_0 %40
         %40 = OpLabel
         %41 = OpCompositeExtract %uint %37 1
         %42 = OpBitwiseAnd %uint %41 %uint_16777215
         %43 = OpAccessChain %_ptr_UniformConstant_type_2d_image %texTable %42
         %44 = OpLoad %type_2d_image %43
         %45 = OpAccessChain %_ptr_Uniform_uint %scene %int_0 %int_0
         %46 = OpLoad %uint %45
         %47 = OpINotEqual %bool %46 %uint_0
               OpSelectionMerge %48 DontFlatten
               OpBranchConditional %47 %49 %50
         %50 = OpLabel
         %51 = OpLoad %type_sampler %mySampler
         %52 = OpSampledImage %type_sampled_image %44 %51
         %53 = OpImageSampleExplicitLod %v4float %52 %21 Lod %float_0
         %54 = OpCompositeExtract %float %53 0
               OpBranch %39
         %49 = OpLabel
               OpBranch %39
         %48 = OpLabel
               OpUnreachable
         %39 = OpLabel
         %55 = OpPhi %float %54 %50 %float_1 %49
         %56 = OpCompositeConstruct %v3float %float_n1 %float_n1 %55
               OpSelectionMerge %57 None
               OpSwitch %uint_0 %58
         %58 = OpLabel
               OpSelectionMerge %59 DontFlatten
               OpBranchConditional %47 %60 %61
         %61 = OpLabel
         %62 = OpLoad %type_sampler %mySampler
         %63 = OpSampledImage %type_sampled_image %44 %62
         %64 = OpImageSampleExplicitLod %v4float %63 %21 Lod %float_0
         %65 = OpCompositeExtract %float %64 0
               OpBranch %57
         %60 = OpLabel
               OpBranch %57
         %59 = OpLabel
               OpUnreachable
         %57 = OpLabel
         %66 = OpPhi %float %65 %61 %float_1 %60
         %67 = OpCompositeConstruct %v3float %float_1 %float_1 %66
         %68 = OpExtInst %v3float %1 Cross %56 %67
         %69 = OpCompositeExtract %float %68 0
         %70 = OpConvertFToU %uint %69
               OpStore %out_var_SV_TARGET1 %70
               OpReturn
               OpFunctionEnd

