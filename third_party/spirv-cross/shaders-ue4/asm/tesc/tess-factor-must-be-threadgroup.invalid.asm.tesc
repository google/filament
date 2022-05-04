; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 179
; Schema: 0
               OpCapability Tessellation
               OpCapability SampledBuffer
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %MainHull "main" %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_VS_To_DS_Position %gl_InvocationID %out_var_TEXCOORD10_centroid %out_var_TEXCOORD11_centroid %out_var_VS_To_DS_Position %out_var_Flat_DisplacementScales %out_var_Flat_TessellationMultiplier %out_var_Flat_WorldDisplacementMultiplier %gl_TessLevelOuter %gl_TessLevelInner
               OpExecutionMode %MainHull Triangles
               OpExecutionMode %MainHull SpacingFractionalOdd
               OpExecutionMode %MainHull VertexOrderCw
               OpExecutionMode %MainHull OutputVertices 3
               OpSource HLSL 600
               OpName %FFlatTessellationHSToDS "FFlatTessellationHSToDS"
               OpMemberName %FFlatTessellationHSToDS 0 "PassSpecificData"
               OpMemberName %FFlatTessellationHSToDS 1 "DisplacementScale"
               OpMemberName %FFlatTessellationHSToDS 2 "TessellationMultiplier"
               OpMemberName %FFlatTessellationHSToDS 3 "WorldDisplacementMultiplier"
               OpName %FBasePassVSToDS "FBasePassVSToDS"
               OpMemberName %FBasePassVSToDS 0 "FactoryInterpolants"
               OpMemberName %FBasePassVSToDS 1 "BasePassInterpolants"
               OpMemberName %FBasePassVSToDS 2 "Position"
               OpName %FVertexFactoryInterpolantsVSToDS "FVertexFactoryInterpolantsVSToDS"
               OpMemberName %FVertexFactoryInterpolantsVSToDS 0 "InterpolantsVSToPS"
               OpName %FVertexFactoryInterpolantsVSToPS "FVertexFactoryInterpolantsVSToPS"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 0 "TangentToWorld0"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 1 "TangentToWorld2"
               OpName %FBasePassInterpolantsVSToDS "FBasePassInterpolantsVSToDS"
               OpName %FSharedBasePassInterpolants "FSharedBasePassInterpolants"
               OpName %type_Primitive "type.Primitive"
               OpMemberName %type_Primitive 0 "Primitive_LocalToWorld"
               OpMemberName %type_Primitive 1 "Primitive_InvNonUniformScaleAndDeterminantSign"
               OpMemberName %type_Primitive 2 "Primitive_ObjectWorldPositionAndRadius"
               OpMemberName %type_Primitive 3 "Primitive_WorldToLocal"
               OpMemberName %type_Primitive 4 "Primitive_PreviousLocalToWorld"
               OpMemberName %type_Primitive 5 "Primitive_PreviousWorldToLocal"
               OpMemberName %type_Primitive 6 "Primitive_ActorWorldPosition"
               OpMemberName %type_Primitive 7 "Primitive_UseSingleSampleShadowFromStationaryLights"
               OpMemberName %type_Primitive 8 "Primitive_ObjectBounds"
               OpMemberName %type_Primitive 9 "Primitive_LpvBiasMultiplier"
               OpMemberName %type_Primitive 10 "Primitive_DecalReceiverMask"
               OpMemberName %type_Primitive 11 "Primitive_PerObjectGBufferData"
               OpMemberName %type_Primitive 12 "Primitive_UseVolumetricLightmapShadowFromStationaryLights"
               OpMemberName %type_Primitive 13 "Primitive_DrawsVelocity"
               OpMemberName %type_Primitive 14 "Primitive_ObjectOrientation"
               OpMemberName %type_Primitive 15 "Primitive_NonUniformScale"
               OpMemberName %type_Primitive 16 "Primitive_LocalObjectBoundsMin"
               OpMemberName %type_Primitive 17 "Primitive_LightingChannelMask"
               OpMemberName %type_Primitive 18 "Primitive_LocalObjectBoundsMax"
               OpMemberName %type_Primitive 19 "Primitive_LightmapDataIndex"
               OpMemberName %type_Primitive 20 "Primitive_PreSkinnedLocalBounds"
               OpMemberName %type_Primitive 21 "Primitive_SingleCaptureIndex"
               OpMemberName %type_Primitive 22 "Primitive_OutputVelocity"
               OpMemberName %type_Primitive 23 "PrePadding_Primitive_420"
               OpMemberName %type_Primitive 24 "PrePadding_Primitive_424"
               OpMemberName %type_Primitive 25 "PrePadding_Primitive_428"
               OpMemberName %type_Primitive 26 "Primitive_CustomPrimitiveData"
               OpName %Primitive "Primitive"
               OpName %type_Material "type.Material"
               OpMemberName %type_Material 0 "Material_VectorExpressions"
               OpMemberName %type_Material 1 "Material_ScalarExpressions"
               OpName %Material "Material"
               OpName %in_var_TEXCOORD10_centroid "in.var.TEXCOORD10_centroid"
               OpName %in_var_TEXCOORD11_centroid "in.var.TEXCOORD11_centroid"
               OpName %in_var_VS_To_DS_Position "in.var.VS_To_DS_Position"
               OpName %out_var_TEXCOORD10_centroid "out.var.TEXCOORD10_centroid"
               OpName %out_var_TEXCOORD11_centroid "out.var.TEXCOORD11_centroid"
               OpName %out_var_VS_To_DS_Position "out.var.VS_To_DS_Position"
               OpName %out_var_Flat_DisplacementScales "out.var.Flat_DisplacementScales"
               OpName %out_var_Flat_TessellationMultiplier "out.var.Flat_TessellationMultiplier"
               OpName %out_var_Flat_WorldDisplacementMultiplier "out.var.Flat_WorldDisplacementMultiplier"
               OpName %MainHull "MainHull"
               OpName %param_var_I "param.var.I"
               OpName %temp_var_hullMainRetVal "temp.var.hullMainRetVal"
               OpName %if_merge "if.merge"
               OpDecorateString %in_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %in_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %in_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorateString %gl_InvocationID UserSemantic "SV_OutputControlPointID"
               OpDecorateString %out_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %out_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %out_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorateString %out_var_Flat_DisplacementScales UserSemantic "Flat_DisplacementScales"
               OpDecorateString %out_var_Flat_TessellationMultiplier UserSemantic "Flat_TessellationMultiplier"
               OpDecorateString %out_var_Flat_WorldDisplacementMultiplier UserSemantic "Flat_WorldDisplacementMultiplier"
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorateString %gl_TessLevelOuter UserSemantic "SV_TessFactor"
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorateString %gl_TessLevelInner UserSemantic "SV_InsideTessFactor"
               OpDecorate %gl_TessLevelInner Patch
               OpDecorate %in_var_TEXCOORD10_centroid Location 0
               OpDecorate %in_var_TEXCOORD11_centroid Location 1
               OpDecorate %in_var_VS_To_DS_Position Location 2
               OpDecorate %out_var_Flat_DisplacementScales Location 0
               OpDecorate %out_var_Flat_TessellationMultiplier Location 1
               OpDecorate %out_var_Flat_WorldDisplacementMultiplier Location 2
               OpDecorate %out_var_TEXCOORD10_centroid Location 3
               OpDecorate %out_var_TEXCOORD11_centroid Location 4
               OpDecorate %out_var_VS_To_DS_Position Location 5
               OpDecorate %Primitive DescriptorSet 0
               OpDecorate %Primitive Binding 0
               OpDecorate %Material DescriptorSet 0
               OpDecorate %Material Binding 1
               OpDecorate %_arr_v4float_uint_4 ArrayStride 16
               OpMemberDecorate %type_Primitive 0 Offset 0
               OpMemberDecorate %type_Primitive 0 MatrixStride 16
               OpMemberDecorate %type_Primitive 0 ColMajor
               OpMemberDecorate %type_Primitive 1 Offset 64
               OpMemberDecorate %type_Primitive 2 Offset 80
               OpMemberDecorate %type_Primitive 3 Offset 96
               OpMemberDecorate %type_Primitive 3 MatrixStride 16
               OpMemberDecorate %type_Primitive 3 ColMajor
               OpMemberDecorate %type_Primitive 4 Offset 160
               OpMemberDecorate %type_Primitive 4 MatrixStride 16
               OpMemberDecorate %type_Primitive 4 ColMajor
               OpMemberDecorate %type_Primitive 5 Offset 224
               OpMemberDecorate %type_Primitive 5 MatrixStride 16
               OpMemberDecorate %type_Primitive 5 ColMajor
               OpMemberDecorate %type_Primitive 6 Offset 288
               OpMemberDecorate %type_Primitive 7 Offset 300
               OpMemberDecorate %type_Primitive 8 Offset 304
               OpMemberDecorate %type_Primitive 9 Offset 316
               OpMemberDecorate %type_Primitive 10 Offset 320
               OpMemberDecorate %type_Primitive 11 Offset 324
               OpMemberDecorate %type_Primitive 12 Offset 328
               OpMemberDecorate %type_Primitive 13 Offset 332
               OpMemberDecorate %type_Primitive 14 Offset 336
               OpMemberDecorate %type_Primitive 15 Offset 352
               OpMemberDecorate %type_Primitive 16 Offset 368
               OpMemberDecorate %type_Primitive 17 Offset 380
               OpMemberDecorate %type_Primitive 18 Offset 384
               OpMemberDecorate %type_Primitive 19 Offset 396
               OpMemberDecorate %type_Primitive 20 Offset 400
               OpMemberDecorate %type_Primitive 21 Offset 412
               OpMemberDecorate %type_Primitive 22 Offset 416
               OpMemberDecorate %type_Primitive 23 Offset 420
               OpMemberDecorate %type_Primitive 24 Offset 424
               OpMemberDecorate %type_Primitive 25 Offset 428
               OpMemberDecorate %type_Primitive 26 Offset 432
               OpDecorate %type_Primitive Block
               OpDecorate %_arr_v4float_uint_3 ArrayStride 16
               OpDecorate %_arr_v4float_uint_1 ArrayStride 16
               OpMemberDecorate %type_Material 0 Offset 0
               OpMemberDecorate %type_Material 1 Offset 48
               OpDecorate %type_Material Block
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
    %v3float = OpTypeVector %float 3
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
  %float_0_5 = OpConstant %float 0.5
      %int_1 = OpConstant %int 1
%float_0_333000004 = OpConstant %float 0.333000004
    %float_1 = OpConstant %float 1
         %49 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
   %float_15 = OpConstant %float 15
         %51 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%FVertexFactoryInterpolantsVSToPS = OpTypeStruct %v4float %v4float
%FVertexFactoryInterpolantsVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToPS
%FSharedBasePassInterpolants = OpTypeStruct
%FBasePassInterpolantsVSToDS = OpTypeStruct %FSharedBasePassInterpolants
%FBasePassVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToDS %FBasePassInterpolantsVSToDS %v4float
%FFlatTessellationHSToDS = OpTypeStruct %FBasePassVSToDS %v3float %float %float
     %int_15 = OpConstant %int 15
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%type_Primitive = OpTypeStruct %mat4v4float %v4float %v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %float %float %float %float %v4float %v4float %v3float %uint %v3float %uint %v3float %int %uint %uint %uint %uint %_arr_v4float_uint_4
%_ptr_Uniform_type_Primitive = OpTypePointer Uniform %type_Primitive
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%type_Material = OpTypeStruct %_arr_v4float_uint_3 %_arr_v4float_uint_1
%_ptr_Uniform_type_Material = OpTypePointer Uniform %type_Material
%_arr_v4float_uint_3_0 = OpTypeArray %v4float %uint_3
%_ptr_Input__arr_v4float_uint_3_0 = OpTypePointer Input %_arr_v4float_uint_3_0
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output__arr_v4float_uint_3_0 = OpTypePointer Output %_arr_v4float_uint_3_0
%_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
%_ptr_Output__arr_v3float_uint_3 = OpTypePointer Output %_arr_v3float_uint_3
%_ptr_Output__arr_float_uint_3 = OpTypePointer Output %_arr_float_uint_3
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
       %void = OpTypeVoid
         %67 = OpTypeFunction %void
%_arr_FBasePassVSToDS_uint_3 = OpTypeArray %FBasePassVSToDS %uint_3
%_ptr_Function__arr_FBasePassVSToDS_uint_3 = OpTypePointer Function %_arr_FBasePassVSToDS_uint_3
%_arr_FFlatTessellationHSToDS_uint_3 = OpTypeArray %FFlatTessellationHSToDS %uint_3
%_ptr_Function__arr_FFlatTessellationHSToDS_uint_3 = OpTypePointer Function %_arr_FFlatTessellationHSToDS_uint_3
%_ptr_Workgroup__arr_FFlatTessellationHSToDS_uint_3 = OpTypePointer Workgroup %_arr_FFlatTessellationHSToDS_uint_3
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output_v3float = OpTypePointer Output %v3float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Function_FFlatTessellationHSToDS = OpTypePointer Function %FFlatTessellationHSToDS
%_ptr_Workgroup_FFlatTessellationHSToDS = OpTypePointer Workgroup %FFlatTessellationHSToDS
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Workgroup_float = OpTypePointer Workgroup %float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_FVertexFactoryInterpolantsVSToDS = OpTypePointer Function %FVertexFactoryInterpolantsVSToDS
%_ptr_Function_FBasePassVSToDS = OpTypePointer Function %FBasePassVSToDS
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
  %Primitive = OpVariable %_ptr_Uniform_type_Primitive Uniform
   %Material = OpVariable %_ptr_Uniform_type_Material Uniform
%in_var_TEXCOORD10_centroid = OpVariable %_ptr_Input__arr_v4float_uint_3_0 Input
%in_var_TEXCOORD11_centroid = OpVariable %_ptr_Input__arr_v4float_uint_3_0 Input
%in_var_VS_To_DS_Position = OpVariable %_ptr_Input__arr_v4float_uint_3_0 Input
%gl_InvocationID = OpVariable %_ptr_Input_uint Input
%out_var_TEXCOORD10_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3_0 Output
%out_var_TEXCOORD11_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3_0 Output
%out_var_VS_To_DS_Position = OpVariable %_ptr_Output__arr_v4float_uint_3_0 Output
%out_var_Flat_DisplacementScales = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%out_var_Flat_TessellationMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%out_var_Flat_WorldDisplacementMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
         %83 = OpConstantNull %FSharedBasePassInterpolants
         %84 = OpConstantComposite %FBasePassInterpolantsVSToDS %83
         %85 = OpUndef %v4float

; XXX: Original asm used Function here, which is wrong.
; This patches the SPIR-V to be correct.
%temp_var_hullMainRetVal = OpVariable %_ptr_Workgroup__arr_FFlatTessellationHSToDS_uint_3 Workgroup

   %MainHull = OpFunction %void None %67
         %86 = OpLabel
%param_var_I = OpVariable %_ptr_Function__arr_FBasePassVSToDS_uint_3 Function
         %87 = OpLoad %_arr_v4float_uint_3_0 %in_var_TEXCOORD10_centroid
         %88 = OpLoad %_arr_v4float_uint_3_0 %in_var_TEXCOORD11_centroid
         %89 = OpCompositeExtract %v4float %87 0
         %90 = OpCompositeExtract %v4float %88 0
         %91 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %89 %90
         %92 = OpCompositeExtract %v4float %87 1
         %93 = OpCompositeExtract %v4float %88 1
         %94 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %92 %93
         %95 = OpCompositeExtract %v4float %87 2
         %96 = OpCompositeExtract %v4float %88 2
         %97 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %95 %96
         %98 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %91
         %99 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %94
        %100 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %97
        %101 = OpLoad %_arr_v4float_uint_3_0 %in_var_VS_To_DS_Position
        %102 = OpCompositeExtract %v4float %101 0
        %103 = OpCompositeConstruct %FBasePassVSToDS %98 %84 %102
        %104 = OpCompositeExtract %v4float %101 1
        %105 = OpCompositeConstruct %FBasePassVSToDS %99 %84 %104
        %106 = OpCompositeExtract %v4float %101 2
        %107 = OpCompositeConstruct %FBasePassVSToDS %100 %84 %106
        %108 = OpCompositeConstruct %_arr_FBasePassVSToDS_uint_3 %103 %105 %107
               OpStore %param_var_I %108
        %109 = OpLoad %uint %gl_InvocationID
        %110 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %109 %int_0
        %111 = OpLoad %FVertexFactoryInterpolantsVSToDS %110
        %112 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %111 0
        %113 = OpCompositeExtract %v4float %112 0
        %114 = OpCompositeExtract %v4float %112 1
        %115 = OpVectorShuffle %v3float %113 %113 0 1 2
        %116 = OpVectorShuffle %v3float %114 %114 0 1 2
        %117 = OpExtInst %v3float %1 Cross %116 %115
        %118 = OpCompositeExtract %float %114 3
        %119 = OpCompositeConstruct %v3float %118 %118 %118
        %120 = OpFMul %v3float %117 %119
        %121 = OpCompositeConstruct %mat3v3float %115 %120 %116
        %122 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_15
        %123 = OpLoad %v4float %122
        %124 = OpVectorShuffle %v3float %123 %123 0 1 2
        %125 = OpVectorTimesMatrix %v3float %124 %121
        %126 = OpAccessChain %_ptr_Function_FBasePassVSToDS %param_var_I %109
        %127 = OpLoad %FBasePassVSToDS %126
        %128 = OpAccessChain %_ptr_Uniform_float %Material %int_1 %int_0 %int_0
        %129 = OpLoad %float %128
        %130 = OpCompositeConstruct %FFlatTessellationHSToDS %127 %125 %129 %float_1
        %131 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %127 0
        %132 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %131 0
        %133 = OpCompositeExtract %v4float %132 0
        %134 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD10_centroid %109
               OpStore %134 %133
        %135 = OpCompositeExtract %v4float %132 1
        %136 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD11_centroid %109
               OpStore %136 %135
        %137 = OpCompositeExtract %v4float %127 2
        %138 = OpAccessChain %_ptr_Output_v4float %out_var_VS_To_DS_Position %109
               OpStore %138 %137
        %139 = OpAccessChain %_ptr_Output_v3float %out_var_Flat_DisplacementScales %109
               OpStore %139 %125
        %140 = OpAccessChain %_ptr_Output_float %out_var_Flat_TessellationMultiplier %109
               OpStore %140 %129
        %141 = OpAccessChain %_ptr_Output_float %out_var_Flat_WorldDisplacementMultiplier %109
               OpStore %141 %float_1
        %142 = OpAccessChain %_ptr_Workgroup_FFlatTessellationHSToDS %temp_var_hullMainRetVal %109
               OpStore %142 %130
               OpControlBarrier %uint_2 %uint_4 %uint_0
        %143 = OpIEqual %bool %109 %uint_0
               OpSelectionMerge %if_merge None
               OpBranchConditional %143 %144 %if_merge
        %144 = OpLabel
        %145 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_1 %int_2
        %146 = OpLoad %float %145
        %147 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_2 %int_2
        %148 = OpLoad %float %147
        %149 = OpFAdd %float %146 %148
        %150 = OpFMul %float %float_0_5 %149
        %151 = OpCompositeInsert %v4float %150 %85 0
        %152 = OpLoad %float %147
        %153 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_0 %int_2
        %154 = OpLoad %float %153
        %155 = OpFAdd %float %152 %154
        %156 = OpFMul %float %float_0_5 %155
        %157 = OpCompositeInsert %v4float %156 %151 1
        %158 = OpLoad %float %153
        %159 = OpLoad %float %145
        %160 = OpFAdd %float %158 %159
        %161 = OpFMul %float %float_0_5 %160
        %162 = OpCompositeInsert %v4float %161 %157 2
        %163 = OpLoad %float %153
        %164 = OpLoad %float %145
        %165 = OpFAdd %float %163 %164
        %166 = OpLoad %float %147
        %167 = OpFAdd %float %165 %166
        %168 = OpFMul %float %float_0_333000004 %167
        %169 = OpCompositeInsert %v4float %168 %162 3
        %170 = OpExtInst %v4float %1 FClamp %169 %49 %51
        %171 = OpCompositeExtract %float %170 0
        %172 = OpCompositeExtract %float %170 1
        %173 = OpCompositeExtract %float %170 2
        %174 = OpCompositeExtract %float %170 3
        %175 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_0
               OpStore %175 %171
        %176 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_1
               OpStore %176 %172
        %177 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_2
               OpStore %177 %173
        %178 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0
               OpStore %178 %174
               OpBranch %if_merge
   %if_merge = OpLabel
               OpReturn
               OpFunctionEnd
