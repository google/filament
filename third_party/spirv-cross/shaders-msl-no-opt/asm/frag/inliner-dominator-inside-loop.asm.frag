; SPIR-V
; Version: 1.0
; Generator: Khronos Glslang Reference Front End; 3
; Bound: 1532
; Schema: 0
               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %IN_HPosition %IN_Uv_EdgeDistance1 %IN_UvStuds_EdgeDistance2 %IN_Color %IN_LightPosition_Fog %IN_View_Depth %IN_Normal_SpecPower %IN_Tangent %IN_PosLightSpace_Reflectance %IN_studIndex %_entryPointOutput
               OpExecutionMode %main OriginUpperLeft
               OpSource HLSL 500
               OpName %main "main"
               OpName %VertexOutput "VertexOutput"
               OpMemberName %VertexOutput 0 "HPosition"
               OpMemberName %VertexOutput 1 "Uv_EdgeDistance1"
               OpMemberName %VertexOutput 2 "UvStuds_EdgeDistance2"
               OpMemberName %VertexOutput 3 "Color"
               OpMemberName %VertexOutput 4 "LightPosition_Fog"
               OpMemberName %VertexOutput 5 "View_Depth"
               OpMemberName %VertexOutput 6 "Normal_SpecPower"
               OpMemberName %VertexOutput 7 "Tangent"
               OpMemberName %VertexOutput 8 "PosLightSpace_Reflectance"
               OpMemberName %VertexOutput 9 "studIndex"
               OpName %Surface "Surface"
               OpMemberName %Surface 0 "albedo"
               OpMemberName %Surface 1 "normal"
               OpMemberName %Surface 2 "specular"
               OpMemberName %Surface 3 "gloss"
               OpMemberName %Surface 4 "reflectance"
               OpMemberName %Surface 5 "opacity"
               OpName %SurfaceInput "SurfaceInput"
               OpMemberName %SurfaceInput 0 "Color"
               OpMemberName %SurfaceInput 1 "Uv"
               OpMemberName %SurfaceInput 2 "UvStuds"
               OpName %Globals "Globals"
               OpMemberName %Globals 0 "ViewProjection"
               OpMemberName %Globals 1 "ViewRight"
               OpMemberName %Globals 2 "ViewUp"
               OpMemberName %Globals 3 "ViewDir"
               OpMemberName %Globals 4 "CameraPosition"
               OpMemberName %Globals 5 "AmbientColor"
               OpMemberName %Globals 6 "Lamp0Color"
               OpMemberName %Globals 7 "Lamp0Dir"
               OpMemberName %Globals 8 "Lamp1Color"
               OpMemberName %Globals 9 "FogParams"
               OpMemberName %Globals 10 "FogColor"
               OpMemberName %Globals 11 "LightBorder"
               OpMemberName %Globals 12 "LightConfig0"
               OpMemberName %Globals 13 "LightConfig1"
               OpMemberName %Globals 14 "LightConfig2"
               OpMemberName %Globals 15 "LightConfig3"
               OpMemberName %Globals 16 "RefractionBias_FadeDistance_GlowFactor"
               OpMemberName %Globals 17 "OutlineBrightness_ShadowInfo"
               OpMemberName %Globals 18 "ShadowMatrix0"
               OpMemberName %Globals 19 "ShadowMatrix1"
               OpMemberName %Globals 20 "ShadowMatrix2"
               OpName %CB0 "CB0"
               OpMemberName %CB0 0 "CB0"
               OpName %_ ""
               OpName %LightMapTexture "LightMapTexture"
               OpName %LightMapSampler "LightMapSampler"
               OpName %ShadowMapSampler "ShadowMapSampler"
               OpName %ShadowMapTexture "ShadowMapTexture"
               OpName %EnvironmentMapTexture "EnvironmentMapTexture"
               OpName %EnvironmentMapSampler "EnvironmentMapSampler"
               OpName %IN_HPosition "IN.HPosition"
               OpName %IN_Uv_EdgeDistance1 "IN.Uv_EdgeDistance1"
               OpName %IN_UvStuds_EdgeDistance2 "IN.UvStuds_EdgeDistance2"
               OpName %IN_Color "IN.Color"
               OpName %IN_LightPosition_Fog "IN.LightPosition_Fog"
               OpName %IN_View_Depth "IN.View_Depth"
               OpName %IN_Normal_SpecPower "IN.Normal_SpecPower"
               OpName %IN_Tangent "IN.Tangent"
               OpName %IN_PosLightSpace_Reflectance "IN.PosLightSpace_Reflectance"
               OpName %IN_studIndex "IN.studIndex"
               OpName %_entryPointOutput "@entryPointOutput"
               OpName %DiffuseMapSampler "DiffuseMapSampler"
               OpName %DiffuseMapTexture "DiffuseMapTexture"
               OpName %NormalMapSampler "NormalMapSampler"
               OpName %NormalMapTexture "NormalMapTexture"
               OpName %NormalDetailMapTexture "NormalDetailMapTexture"
               OpName %NormalDetailMapSampler "NormalDetailMapSampler"
               OpName %StudsMapTexture "StudsMapTexture"
               OpName %StudsMapSampler "StudsMapSampler"
               OpName %SpecularMapSampler "SpecularMapSampler"
               OpName %SpecularMapTexture "SpecularMapTexture"
               OpName %Params "Params"
               OpMemberName %Params 0 "LqmatFarTilingFactor"
               OpName %CB2 "CB2"
               OpMemberName %CB2 0 "CB2"
               OpMemberDecorate %Globals 0 ColMajor
               OpMemberDecorate %Globals 0 Offset 0
               OpMemberDecorate %Globals 0 MatrixStride 16
               OpMemberDecorate %Globals 1 Offset 64
               OpMemberDecorate %Globals 2 Offset 80
               OpMemberDecorate %Globals 3 Offset 96
               OpMemberDecorate %Globals 4 Offset 112
               OpMemberDecorate %Globals 5 Offset 128
               OpMemberDecorate %Globals 6 Offset 144
               OpMemberDecorate %Globals 7 Offset 160
               OpMemberDecorate %Globals 8 Offset 176
               OpMemberDecorate %Globals 9 Offset 192
               OpMemberDecorate %Globals 10 Offset 208
               OpMemberDecorate %Globals 11 Offset 224
               OpMemberDecorate %Globals 12 Offset 240
               OpMemberDecorate %Globals 13 Offset 256
               OpMemberDecorate %Globals 14 Offset 272
               OpMemberDecorate %Globals 15 Offset 288
               OpMemberDecorate %Globals 16 Offset 304
               OpMemberDecorate %Globals 17 Offset 320
               OpMemberDecorate %Globals 18 Offset 336
               OpMemberDecorate %Globals 19 Offset 352
               OpMemberDecorate %Globals 20 Offset 368
               OpMemberDecorate %CB0 0 Offset 0
               OpDecorate %CB0 Block
               OpDecorate %_ DescriptorSet 0
               OpDecorate %_ Binding 0
               OpDecorate %LightMapTexture DescriptorSet 1
               OpDecorate %LightMapTexture Binding 6
               OpDecorate %LightMapSampler DescriptorSet 1
               OpDecorate %LightMapSampler Binding 6
               OpDecorate %ShadowMapSampler DescriptorSet 1
               OpDecorate %ShadowMapSampler Binding 1
               OpDecorate %ShadowMapTexture DescriptorSet 1
               OpDecorate %ShadowMapTexture Binding 1
               OpDecorate %EnvironmentMapTexture DescriptorSet 1
               OpDecorate %EnvironmentMapTexture Binding 2
               OpDecorate %EnvironmentMapSampler DescriptorSet 1
               OpDecorate %EnvironmentMapSampler Binding 2
               OpDecorate %IN_HPosition BuiltIn FragCoord
               OpDecorate %IN_Uv_EdgeDistance1 Location 0
               OpDecorate %IN_UvStuds_EdgeDistance2 Location 1
               OpDecorate %IN_Color Location 2
               OpDecorate %IN_LightPosition_Fog Location 3
               OpDecorate %IN_View_Depth Location 4
               OpDecorate %IN_Normal_SpecPower Location 5
               OpDecorate %IN_Tangent Location 6
               OpDecorate %IN_PosLightSpace_Reflectance Location 7
               OpDecorate %IN_studIndex Location 8
               OpDecorate %_entryPointOutput Location 0
               OpDecorate %DiffuseMapSampler DescriptorSet 1
               OpDecorate %DiffuseMapSampler Binding 3
               OpDecorate %DiffuseMapTexture DescriptorSet 1
               OpDecorate %DiffuseMapTexture Binding 3
               OpDecorate %NormalMapSampler DescriptorSet 1
               OpDecorate %NormalMapSampler Binding 4
               OpDecorate %NormalMapTexture DescriptorSet 1
               OpDecorate %NormalMapTexture Binding 4
               OpDecorate %NormalDetailMapTexture DescriptorSet 1
               OpDecorate %NormalDetailMapTexture Binding 8
               OpDecorate %NormalDetailMapSampler DescriptorSet 1
               OpDecorate %NormalDetailMapSampler Binding 8
               OpDecorate %StudsMapTexture DescriptorSet 1
               OpDecorate %StudsMapTexture Binding 0
               OpDecorate %StudsMapSampler DescriptorSet 1
               OpDecorate %StudsMapSampler Binding 0
               OpDecorate %SpecularMapSampler DescriptorSet 1
               OpDecorate %SpecularMapSampler Binding 5
               OpDecorate %SpecularMapTexture DescriptorSet 1
               OpDecorate %SpecularMapTexture Binding 5
               OpMemberDecorate %Params 0 Offset 0
               OpMemberDecorate %CB2 0 Offset 0
               OpDecorate %CB2 Block
       %void = OpTypeVoid
          %3 = OpTypeFunction %void
      %float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
          %8 = OpTypeFunction %float %_ptr_Function_float
    %v4float = OpTypeVector %float 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %v3float = OpTypeVector %float 3
         %18 = OpTypeFunction %v3float %_ptr_Function_v4float
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %23 = OpTypeFunction %v4float %_ptr_Function_v3float
         %27 = OpTypeFunction %float %_ptr_Function_v3float
         %31 = OpTypeFunction %float %_ptr_Function_float %_ptr_Function_float
         %36 = OpTypeSampler
%_ptr_Function_36 = OpTypePointer Function %36
         %38 = OpTypeImage %float 2D 0 0 0 1 Unknown
%_ptr_Function_38 = OpTypePointer Function %38
         %40 = OpTypeFunction %float %_ptr_Function_36 %_ptr_Function_38 %_ptr_Function_v3float %_ptr_Function_float
%VertexOutput = OpTypeStruct %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v3float %v4float %float
%_ptr_Function_VertexOutput = OpTypePointer Function %VertexOutput
    %Surface = OpTypeStruct %v3float %v3float %float %float %float %float
         %50 = OpTypeFunction %Surface %_ptr_Function_VertexOutput
         %54 = OpTypeFunction %v4float %_ptr_Function_VertexOutput
    %v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %60 = OpTypeFunction %v4float %_ptr_Function_36 %_ptr_Function_38 %_ptr_Function_v2float %_ptr_Function_float %_ptr_Function_float
%SurfaceInput = OpTypeStruct %v4float %v2float %v2float
%_ptr_Function_SurfaceInput = OpTypePointer Function %SurfaceInput
         %70 = OpTypeFunction %Surface %_ptr_Function_SurfaceInput %_ptr_Function_v2float
    %float_0 = OpConstant %float 0
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
%mat4v4float = OpTypeMatrix %v4float 4
    %Globals = OpTypeStruct %mat4v4float %v4float %v4float %v4float %v3float %v3float %v3float %v3float %v3float %v4float %v3float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float
        %CB0 = OpTypeStruct %Globals
%_ptr_Uniform_CB0 = OpTypePointer Uniform %CB0
          %_ = OpVariable %_ptr_Uniform_CB0 Uniform
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %int_15 = OpConstant %int 15
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
     %int_14 = OpConstant %int 14
        %128 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %133 = OpTypeImage %float 3D 0 0 0 1 Unknown
%_ptr_UniformConstant_133 = OpTypePointer UniformConstant %133
%LightMapTexture = OpVariable %_ptr_UniformConstant_133 UniformConstant
%_ptr_UniformConstant_36 = OpTypePointer UniformConstant %36
%LightMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
        %140 = OpTypeSampledImage %133
     %int_11 = OpConstant %int 11
       %uint = OpTypeInt 32 0
    %float_9 = OpConstant %float 9
   %float_20 = OpConstant %float 20
  %float_0_5 = OpConstant %float 0.5
        %183 = OpTypeSampledImage %38
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %int_17 = OpConstant %int 17
     %uint_3 = OpConstant %uint 3
%_ptr_Uniform_float = OpTypePointer Uniform %float
 %float_0_25 = OpConstant %float 0.25
      %int_5 = OpConstant %int 5
%float_0_00333333 = OpConstant %float 0.00333333
     %int_16 = OpConstant %int 16
%_ptr_Function_Surface = OpTypePointer Function %Surface
      %int_6 = OpConstant %int 6
      %int_7 = OpConstant %int 7
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
      %int_8 = OpConstant %int 8
%ShadowMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
%_ptr_UniformConstant_38 = OpTypePointer UniformConstant %38
%ShadowMapTexture = OpVariable %_ptr_UniformConstant_38 UniformConstant
        %367 = OpTypeImage %float Cube 0 0 0 1 Unknown
%_ptr_UniformConstant_367 = OpTypePointer UniformConstant %367
%EnvironmentMapTexture = OpVariable %_ptr_UniformConstant_367 UniformConstant
%EnvironmentMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
        %373 = OpTypeSampledImage %367
  %float_1_5 = OpConstant %float 1.5
     %int_10 = OpConstant %int 10
%_ptr_Input_v4float = OpTypePointer Input %v4float
%IN_HPosition = OpVariable %_ptr_Input_v4float Input
%IN_Uv_EdgeDistance1 = OpVariable %_ptr_Input_v4float Input
%IN_UvStuds_EdgeDistance2 = OpVariable %_ptr_Input_v4float Input
   %IN_Color = OpVariable %_ptr_Input_v4float Input
%IN_LightPosition_Fog = OpVariable %_ptr_Input_v4float Input
%IN_View_Depth = OpVariable %_ptr_Input_v4float Input
%IN_Normal_SpecPower = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_v3float = OpTypePointer Input %v3float
 %IN_Tangent = OpVariable %_ptr_Input_v3float Input
%IN_PosLightSpace_Reflectance = OpVariable %_ptr_Input_v4float Input
%_ptr_Input_float = OpTypePointer Input %float
%IN_studIndex = OpVariable %_ptr_Input_float Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_entryPointOutput = OpVariable %_ptr_Output_v4float Output
       %bool = OpTypeBool
%DiffuseMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
%DiffuseMapTexture = OpVariable %_ptr_UniformConstant_38 UniformConstant
%NormalMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
%NormalMapTexture = OpVariable %_ptr_UniformConstant_38 UniformConstant
%NormalDetailMapTexture = OpVariable %_ptr_UniformConstant_38 UniformConstant
%NormalDetailMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
  %float_0_3 = OpConstant %float 0.3
%StudsMapTexture = OpVariable %_ptr_UniformConstant_38 UniformConstant
%StudsMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
%SpecularMapSampler = OpVariable %_ptr_UniformConstant_36 UniformConstant
%SpecularMapTexture = OpVariable %_ptr_UniformConstant_38 UniformConstant
 %float_0_75 = OpConstant %float 0.75
  %float_256 = OpConstant %float 256
        %689 = OpConstantComposite %v2float %float_2 %float_256
 %float_0_01 = OpConstant %float 0.01
        %692 = OpConstantComposite %v2float %float_0 %float_0_01
  %float_0_8 = OpConstant %float 0.8
  %float_120 = OpConstant %float 120
        %697 = OpConstantComposite %v2float %float_0_8 %float_120
     %Params = OpTypeStruct %v4float
        %CB2 = OpTypeStruct %Params
%_ptr_Uniform_CB2 = OpTypePointer Uniform %CB2
      %false = OpConstantFalse %bool
       %1509 = OpUndef %VertexOutput
       %1510 = OpUndef %SurfaceInput
       %1511 = OpUndef %v2float
       %1512 = OpUndef %v4float
       %1531 = OpUndef %Surface
       %main = OpFunction %void None %3
          %5 = OpLabel
        %501 = OpLoad %v4float %IN_HPosition
       %1378 = OpCompositeInsert %VertexOutput %501 %1509 0
        %504 = OpLoad %v4float %IN_Uv_EdgeDistance1
       %1380 = OpCompositeInsert %VertexOutput %504 %1378 1
        %507 = OpLoad %v4float %IN_UvStuds_EdgeDistance2
       %1382 = OpCompositeInsert %VertexOutput %507 %1380 2
        %510 = OpLoad %v4float %IN_Color
       %1384 = OpCompositeInsert %VertexOutput %510 %1382 3
        %513 = OpLoad %v4float %IN_LightPosition_Fog
       %1386 = OpCompositeInsert %VertexOutput %513 %1384 4
        %516 = OpLoad %v4float %IN_View_Depth
       %1388 = OpCompositeInsert %VertexOutput %516 %1386 5
        %519 = OpLoad %v4float %IN_Normal_SpecPower
       %1390 = OpCompositeInsert %VertexOutput %519 %1388 6
        %523 = OpLoad %v3float %IN_Tangent
       %1392 = OpCompositeInsert %VertexOutput %523 %1390 7
        %526 = OpLoad %v4float %IN_PosLightSpace_Reflectance
       %1394 = OpCompositeInsert %VertexOutput %526 %1392 8
        %530 = OpLoad %float %IN_studIndex
       %1396 = OpCompositeInsert %VertexOutput %530 %1394 9
       %1400 = OpCompositeInsert %SurfaceInput %510 %1510 0
        %954 = OpVectorShuffle %v2float %504 %504 0 1
       %1404 = OpCompositeInsert %SurfaceInput %954 %1400 1
        %958 = OpVectorShuffle %v2float %507 %507 0 1
       %1408 = OpCompositeInsert %SurfaceInput %958 %1404 2
       %1410 = OpCompositeExtract %float %1408 2 1
        %962 = OpExtInst %float %1 Fract %1410
        %965 = OpFAdd %float %962 %530
        %966 = OpFMul %float %965 %float_0_25
       %1414 = OpCompositeInsert %SurfaceInput %966 %1408 2 1
       %1416 = OpCompositeExtract %float %1396 5 3
        %970 = OpFMul %float %1416 %float_0_00333333
        %971 = OpFSub %float %float_1 %970
        %987 = OpExtInst %float %1 FClamp %971 %float_0 %float_1
        %976 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_16 %uint_1
        %977 = OpLoad %float %976
        %978 = OpFMul %float %1416 %977
        %979 = OpFSub %float %float_1 %978
        %990 = OpExtInst %float %1 FClamp %979 %float_0 %float_1
       %1024 = OpVectorTimesScalar %v2float %954 %float_1
       %1029 = OpLoad %36 %DiffuseMapSampler
       %1030 = OpLoad %38 %DiffuseMapTexture
               OpBranch %1119
       %1119 = OpLabel
               OpLoopMerge %1120 %1121 None
               OpBranch %1122
       %1122 = OpLabel
       %1124 = OpFOrdEqual %bool %float_0 %float_0
               OpSelectionMerge %1125 None
               OpBranchConditional %1124 %1126 %1127
       %1126 = OpLabel
       %1130 = OpSampledImage %183 %1030 %1029
       %1132 = OpImageSampleImplicitLod %v4float %1130 %1024
               OpBranch %1120
       %1127 = OpLabel
       %1134 = OpFSub %float %float_1 %float_0
       %1135 = OpFDiv %float %float_1 %1134
       %1138 = OpSampledImage %183 %1030 %1029
       %1140 = OpVectorTimesScalar %v2float %1024 %float_0_25
       %1141 = OpImageSampleImplicitLod %v4float %1138 %1140
       %1144 = OpSampledImage %183 %1030 %1029
       %1146 = OpImageSampleImplicitLod %v4float %1144 %1024
       %1149 = OpFMul %float %987 %1135
       %1152 = OpFMul %float %float_0 %1135
       %1153 = OpFSub %float %1149 %1152
       %1161 = OpExtInst %float %1 FClamp %1153 %float_0 %float_1
       %1155 = OpCompositeConstruct %v4float %1161 %1161 %1161 %1161
       %1156 = OpExtInst %v4float %1 FMix %1141 %1146 %1155
               OpBranch %1120
       %1125 = OpLabel
       %1157 = OpUndef %v4float
               OpBranch %1120
       %1121 = OpLabel
               OpBranchConditional %false %1119 %1120
       %1120 = OpLabel
       %1517 = OpPhi %v4float %1132 %1126 %1156 %1127 %1157 %1125 %1512 %1121
       %1035 = OpVectorTimesScalar %v4float %1517 %float_1
       %1036 = OpLoad %36 %NormalMapSampler
       %1037 = OpLoad %38 %NormalMapTexture
               OpBranch %1165
       %1165 = OpLabel
               OpLoopMerge %1166 %1167 None
               OpBranch %1168
       %1168 = OpLabel
               OpSelectionMerge %1171 None
               OpBranchConditional %1124 %1172 %1173
       %1172 = OpLabel
       %1176 = OpSampledImage %183 %1037 %1036
       %1178 = OpImageSampleImplicitLod %v4float %1176 %1024
               OpBranch %1166
       %1173 = OpLabel
       %1180 = OpFSub %float %float_1 %float_0
       %1181 = OpFDiv %float %float_1 %1180
       %1184 = OpSampledImage %183 %1037 %1036
       %1186 = OpVectorTimesScalar %v2float %1024 %float_0_25
       %1187 = OpImageSampleImplicitLod %v4float %1184 %1186
       %1190 = OpSampledImage %183 %1037 %1036
       %1192 = OpImageSampleImplicitLod %v4float %1190 %1024
       %1195 = OpFMul %float %990 %1181
       %1198 = OpFMul %float %float_0 %1181
       %1199 = OpFSub %float %1195 %1198
       %1206 = OpExtInst %float %1 FClamp %1199 %float_0 %float_1
       %1201 = OpCompositeConstruct %v4float %1206 %1206 %1206 %1206
       %1202 = OpExtInst %v4float %1 FMix %1187 %1192 %1201
               OpBranch %1166
       %1171 = OpLabel
       %1203 = OpUndef %v4float
               OpBranch %1166
       %1167 = OpLabel
               OpBranchConditional %false %1165 %1166
       %1166 = OpLabel
       %1523 = OpPhi %v4float %1178 %1172 %1202 %1173 %1203 %1171 %1512 %1167
       %1210 = OpVectorShuffle %v2float %1523 %1523 3 1
       %1211 = OpVectorTimesScalar %v2float %1210 %float_2
       %1212 = OpCompositeConstruct %v2float %float_1 %float_1
       %1213 = OpFSub %v2float %1211 %1212
       %1216 = OpFNegate %v2float %1213
       %1218 = OpDot %float %1216 %1213
       %1219 = OpFAdd %float %float_1 %1218
       %1220 = OpExtInst %float %1 FClamp %1219 %float_0 %float_1
       %1221 = OpExtInst %float %1 Sqrt %1220
       %1222 = OpCompositeExtract %float %1213 0
       %1223 = OpCompositeExtract %float %1213 1
       %1224 = OpCompositeConstruct %v3float %1222 %1223 %1221
       %1042 = OpLoad %38 %NormalDetailMapTexture
       %1043 = OpLoad %36 %NormalDetailMapSampler
       %1044 = OpSampledImage %183 %1042 %1043
       %1046 = OpVectorTimesScalar %v2float %1024 %float_0
       %1047 = OpImageSampleImplicitLod %v4float %1044 %1046
       %1228 = OpVectorShuffle %v2float %1047 %1047 3 1
       %1229 = OpVectorTimesScalar %v2float %1228 %float_2
       %1231 = OpFSub %v2float %1229 %1212
       %1234 = OpFNegate %v2float %1231
       %1236 = OpDot %float %1234 %1231
       %1237 = OpFAdd %float %float_1 %1236
       %1238 = OpExtInst %float %1 FClamp %1237 %float_0 %float_1
       %1239 = OpExtInst %float %1 Sqrt %1238
       %1240 = OpCompositeExtract %float %1231 0
       %1241 = OpCompositeExtract %float %1231 1
       %1242 = OpCompositeConstruct %v3float %1240 %1241 %1239
       %1050 = OpVectorShuffle %v2float %1242 %1242 0 1
       %1051 = OpVectorTimesScalar %v2float %1050 %float_0
       %1053 = OpVectorShuffle %v2float %1224 %1224 0 1
       %1054 = OpFAdd %v2float %1053 %1051
       %1056 = OpVectorShuffle %v3float %1224 %1054 3 4 2
       %1059 = OpVectorShuffle %v2float %1056 %1056 0 1
       %1060 = OpVectorTimesScalar %v2float %1059 %990
       %1062 = OpVectorShuffle %v3float %1056 %1060 3 4 2
       %1430 = OpCompositeExtract %float %1062 0
       %1065 = OpFMul %float %1430 %float_0_3
       %1066 = OpFAdd %float %float_1 %1065
       %1069 = OpVectorShuffle %v3float %510 %510 0 1 2
       %1071 = OpVectorShuffle %v3float %1035 %1035 0 1 2
       %1072 = OpFMul %v3float %1069 %1071
       %1074 = OpVectorTimesScalar %v3float %1072 %1066
       %1075 = OpLoad %38 %StudsMapTexture
       %1076 = OpLoad %36 %StudsMapSampler
       %1077 = OpSampledImage %183 %1075 %1076
       %1434 = OpCompositeExtract %v2float %1414 2
       %1080 = OpImageSampleImplicitLod %v4float %1077 %1434
       %1436 = OpCompositeExtract %float %1080 0
       %1083 = OpFMul %float %1436 %float_2
       %1085 = OpVectorTimesScalar %v3float %1074 %1083
       %1086 = OpLoad %36 %SpecularMapSampler
       %1087 = OpLoad %38 %SpecularMapTexture
               OpBranch %1246
       %1246 = OpLabel
               OpLoopMerge %1247 %1248 None
               OpBranch %1249
       %1249 = OpLabel
       %1251 = OpFOrdEqual %bool %float_0_75 %float_0
               OpSelectionMerge %1252 None
               OpBranchConditional %1251 %1253 %1254
       %1253 = OpLabel
       %1257 = OpSampledImage %183 %1087 %1086
       %1259 = OpImageSampleImplicitLod %v4float %1257 %1024
               OpBranch %1247
       %1254 = OpLabel
       %1261 = OpFSub %float %float_1 %float_0_75
       %1262 = OpFDiv %float %float_1 %1261
       %1265 = OpSampledImage %183 %1087 %1086
       %1267 = OpVectorTimesScalar %v2float %1024 %float_0_25
       %1268 = OpImageSampleImplicitLod %v4float %1265 %1267
       %1271 = OpSampledImage %183 %1087 %1086
       %1273 = OpImageSampleImplicitLod %v4float %1271 %1024
       %1276 = OpFMul %float %990 %1262
       %1279 = OpFMul %float %float_0_75 %1262
       %1280 = OpFSub %float %1276 %1279
       %1287 = OpExtInst %float %1 FClamp %1280 %float_0 %float_1
       %1282 = OpCompositeConstruct %v4float %1287 %1287 %1287 %1287
       %1283 = OpExtInst %v4float %1 FMix %1268 %1273 %1282
               OpBranch %1247
       %1252 = OpLabel
       %1284 = OpUndef %v4float
               OpBranch %1247
       %1248 = OpLabel
               OpBranchConditional %false %1246 %1247
       %1247 = OpLabel
       %1530 = OpPhi %v4float %1259 %1253 %1283 %1254 %1284 %1252 %1512 %1248
       %1091 = OpVectorShuffle %v2float %1530 %1530 0 1
       %1093 = OpFMul %v2float %1091 %689
       %1094 = OpFAdd %v2float %1093 %692
       %1097 = OpCompositeConstruct %v2float %990 %990
       %1098 = OpExtInst %v2float %1 FMix %697 %1094 %1097
       %1438 = OpCompositeInsert %Surface %1085 %1531 0
       %1440 = OpCompositeInsert %Surface %1062 %1438 1
       %1442 = OpCompositeExtract %float %1098 0
       %1444 = OpCompositeInsert %Surface %1442 %1440 2
       %1446 = OpCompositeExtract %float %1098 1
       %1448 = OpCompositeInsert %Surface %1446 %1444 3
       %1450 = OpCompositeExtract %float %1091 1
       %1112 = OpFMul %float %1450 %990
       %1113 = OpFMul %float %1112 %float_0
       %1452 = OpCompositeInsert %Surface %1113 %1448 4
       %1456 = OpCompositeExtract %float %1396 3 3
        %764 = OpCompositeExtract %float %1085 0
        %765 = OpCompositeExtract %float %1085 1
        %766 = OpCompositeExtract %float %1085 2
        %767 = OpCompositeConstruct %v4float %764 %765 %766 %1456
        %770 = OpVectorShuffle %v3float %519 %519 0 1 2
        %773 = OpExtInst %v3float %1 Cross %770 %523
       %1462 = OpCompositeExtract %float %1452 1 0
        %778 = OpVectorTimesScalar %v3float %523 %1462
       %1466 = OpCompositeExtract %float %1452 1 1
        %782 = OpVectorTimesScalar %v3float %773 %1466
        %783 = OpFAdd %v3float %778 %782
       %1468 = OpCompositeExtract %float %1452 1 2
        %789 = OpVectorTimesScalar %v3float %770 %1468
        %790 = OpFAdd %v3float %783 %789
        %791 = OpExtInst %v3float %1 Normalize %790
        %793 = OpAccessChain %_ptr_Uniform_v3float %_ %int_0 %int_7
        %794 = OpLoad %v3float %793
        %795 = OpFNegate %v3float %794
        %796 = OpDot %float %791 %795
       %1290 = OpExtInst %float %1 FClamp %796 %float_0 %float_1
        %799 = OpAccessChain %_ptr_Uniform_v3float %_ %int_0 %int_6
        %800 = OpLoad %v3float %799
        %801 = OpVectorTimesScalar %v3float %800 %1290
        %803 = OpFNegate %float %796
        %804 = OpExtInst %float %1 FMax %803 %float_0
        %805 = OpAccessChain %_ptr_Uniform_v3float %_ %int_0 %int_8
        %806 = OpLoad %v3float %805
        %807 = OpVectorTimesScalar %v3float %806 %804
        %808 = OpFAdd %v3float %801 %807
        %810 = OpExtInst %float %1 Step %float_0 %796
        %813 = OpFMul %float %810 %1442
        %820 = OpVectorShuffle %v3float %513 %513 0 1 2
       %1296 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %int_15
       %1297 = OpLoad %v4float %1296
       %1298 = OpVectorShuffle %v3float %1297 %1297 0 1 2
       %1300 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %int_14
       %1301 = OpLoad %v4float %1300
       %1302 = OpVectorShuffle %v3float %1301 %1301 0 1 2
       %1303 = OpFSub %v3float %820 %1302
       %1304 = OpExtInst %v3float %1 FAbs %1303
       %1305 = OpExtInst %v3float %1 Step %1298 %1304
       %1307 = OpDot %float %1305 %128
       %1328 = OpExtInst %float %1 FClamp %1307 %float_0 %float_1
       %1309 = OpLoad %133 %LightMapTexture
       %1310 = OpLoad %36 %LightMapSampler
       %1311 = OpSampledImage %140 %1309 %1310
       %1313 = OpVectorShuffle %v3float %820 %820 1 2 0
       %1317 = OpVectorTimesScalar %v3float %1313 %1328
       %1318 = OpFSub %v3float %1313 %1317
       %1319 = OpImageSampleImplicitLod %v4float %1311 %1318
       %1321 = OpAccessChain %_ptr_Uniform_v4float %_ %int_0 %int_11
       %1322 = OpLoad %v4float %1321
       %1324 = OpCompositeConstruct %v4float %1328 %1328 %1328 %1328
       %1325 = OpExtInst %v4float %1 FMix %1319 %1322 %1324
        %822 = OpLoad %36 %ShadowMapSampler
        %823 = OpLoad %38 %ShadowMapTexture
        %826 = OpVectorShuffle %v3float %526 %526 0 1 2
       %1482 = OpCompositeExtract %float %1325 3
       %1337 = OpSampledImage %183 %823 %822
       %1339 = OpVectorShuffle %v2float %826 %826 0 1
       %1340 = OpImageSampleImplicitLod %v4float %1337 %1339
       %1341 = OpVectorShuffle %v2float %1340 %1340 0 1
       %1484 = OpCompositeExtract %float %826 2
       %1486 = OpCompositeExtract %float %1341 0
       %1363 = OpExtInst %float %1 Step %1486 %1484
       %1365 = OpFSub %float %1484 %float_0_5
       %1366 = OpExtInst %float %1 FAbs %1365
       %1367 = OpFMul %float %float_20 %1366
       %1368 = OpFSub %float %float_9 %1367
       %1369 = OpExtInst %float %1 FClamp %1368 %float_0 %float_1
       %1370 = OpFMul %float %1363 %1369
       %1488 = OpCompositeExtract %float %1341 1
       %1350 = OpFMul %float %1370 %1488
       %1351 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_17 %uint_3
       %1352 = OpLoad %float %1351
       %1353 = OpFMul %float %1350 %1352
       %1354 = OpFSub %float %float_1 %1353
       %1356 = OpFMul %float %1354 %1482
        %830 = OpLoad %367 %EnvironmentMapTexture
        %831 = OpLoad %36 %EnvironmentMapSampler
        %832 = OpSampledImage %373 %830 %831
        %835 = OpVectorShuffle %v3float %516 %516 0 1 2
        %836 = OpFNegate %v3float %835
        %838 = OpExtInst %v3float %1 Reflect %836 %791
        %839 = OpImageSampleImplicitLod %v4float %832 %838
        %840 = OpVectorShuffle %v3float %839 %839 0 1 2
        %842 = OpVectorShuffle %v3float %767 %767 0 1 2
        %845 = OpCompositeConstruct %v3float %1113 %1113 %1113
        %846 = OpExtInst %v3float %1 FMix %842 %840 %845
        %848 = OpVectorShuffle %v4float %767 %846 4 5 6 3
        %849 = OpAccessChain %_ptr_Uniform_v3float %_ %int_0 %int_5
        %850 = OpLoad %v3float %849
        %853 = OpVectorTimesScalar %v3float %808 %1356
        %854 = OpFAdd %v3float %850 %853
        %856 = OpVectorShuffle %v3float %1325 %1325 0 1 2
        %857 = OpFAdd %v3float %854 %856
        %859 = OpVectorShuffle %v3float %848 %848 0 1 2
        %860 = OpFMul %v3float %857 %859
        %865 = OpFMul %float %813 %1356
        %873 = OpExtInst %v3float %1 Normalize %835
        %874 = OpFAdd %v3float %795 %873
        %875 = OpExtInst %v3float %1 Normalize %874
        %876 = OpDot %float %791 %875
        %877 = OpExtInst %float %1 FClamp %876 %float_0 %float_1
        %879 = OpExtInst %float %1 Pow %877 %1446
        %880 = OpFMul %float %865 %879
        %881 = OpVectorTimesScalar %v3float %800 %880
        %884 = OpFAdd %v3float %860 %881
        %886 = OpVectorShuffle %v4float %1512 %884 4 5 6 3
       %1494 = OpCompositeExtract %float %848 3
       %1496 = OpCompositeInsert %v4float %1494 %886 3
        %896 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_17 %uint_0
        %897 = OpLoad %float %896
        %898 = OpFMul %float %978 %897
        %899 = OpAccessChain %_ptr_Uniform_float %_ %int_0 %int_17 %uint_1
        %900 = OpLoad %float %899
        %901 = OpFAdd %float %898 %900
       %1373 = OpExtInst %float %1 FClamp %901 %float_0 %float_1
        %905 = OpVectorShuffle %v2float %504 %504 3 2
        %908 = OpVectorShuffle %v2float %507 %507 3 2
        %909 = OpExtInst %v2float %1 FMin %905 %908
       %1504 = OpCompositeExtract %float %909 0
       %1506 = OpCompositeExtract %float %909 1
        %914 = OpExtInst %float %1 FMin %1504 %1506
        %916 = OpFDiv %float %914 %978
        %919 = OpFSub %float %float_1_5 %916
        %920 = OpFMul %float %1373 %919
        %922 = OpFAdd %float %920 %916
       %1376 = OpExtInst %float %1 FClamp %922 %float_0 %float_1
        %925 = OpVectorShuffle %v3float %1496 %1496 0 1 2
        %926 = OpVectorTimesScalar %v3float %925 %1376
        %928 = OpVectorShuffle %v4float %1496 %926 4 5 6 3
       %1508 = OpCompositeExtract %float %1396 4 3
        %931 = OpExtInst %float %1 FClamp %1508 %float_0 %float_1
        %932 = OpAccessChain %_ptr_Uniform_v3float %_ %int_0 %int_10
        %933 = OpLoad %v3float %932
        %935 = OpVectorShuffle %v3float %928 %928 0 1 2
        %937 = OpCompositeConstruct %v3float %931 %931 %931
        %938 = OpExtInst %v3float %1 FMix %933 %935 %937
        %940 = OpVectorShuffle %v4float %928 %938 4 5 6 3
               OpStore %_entryPointOutput %940
               OpReturn
               OpFunctionEnd
