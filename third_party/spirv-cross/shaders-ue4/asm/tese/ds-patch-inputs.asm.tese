; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 236
; Schema: 0
               OpCapability Tessellation
               OpCapability SampledBuffer
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationEvaluation %MainDomain "main" %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_VS_to_DS_Position %in_var_VS_To_DS_VertexID %in_var_PN_POSITION %in_var_PN_DisplacementScales %in_var_PN_TessellationMultiplier %in_var_PN_WorldDisplacementMultiplier %in_var_PN_DominantVertex %in_var_PN_DominantVertex1 %in_var_PN_DominantVertex2 %in_var_PN_DominantEdge %in_var_PN_DominantEdge1 %in_var_PN_DominantEdge2 %in_var_PN_DominantEdge3 %in_var_PN_DominantEdge4 %in_var_PN_DominantEdge5 %gl_TessLevelOuter %gl_TessLevelInner %in_var_PN_POSITION9 %gl_TessCoord %out_var_TEXCOORD10_centroid %out_var_TEXCOORD11_centroid %out_var_TEXCOORD6 %out_var_TEXCOORD7 %gl_Position
               OpExecutionMode %MainDomain Triangles
               OpSource HLSL 600
               OpName %type_ShadowDepthPass "type.ShadowDepthPass"
               OpMemberName %type_ShadowDepthPass 0 "PrePadding_ShadowDepthPass_LPV_0"
               OpMemberName %type_ShadowDepthPass 1 "PrePadding_ShadowDepthPass_LPV_4"
               OpMemberName %type_ShadowDepthPass 2 "PrePadding_ShadowDepthPass_LPV_8"
               OpMemberName %type_ShadowDepthPass 3 "PrePadding_ShadowDepthPass_LPV_12"
               OpMemberName %type_ShadowDepthPass 4 "PrePadding_ShadowDepthPass_LPV_16"
               OpMemberName %type_ShadowDepthPass 5 "PrePadding_ShadowDepthPass_LPV_20"
               OpMemberName %type_ShadowDepthPass 6 "PrePadding_ShadowDepthPass_LPV_24"
               OpMemberName %type_ShadowDepthPass 7 "PrePadding_ShadowDepthPass_LPV_28"
               OpMemberName %type_ShadowDepthPass 8 "PrePadding_ShadowDepthPass_LPV_32"
               OpMemberName %type_ShadowDepthPass 9 "PrePadding_ShadowDepthPass_LPV_36"
               OpMemberName %type_ShadowDepthPass 10 "PrePadding_ShadowDepthPass_LPV_40"
               OpMemberName %type_ShadowDepthPass 11 "PrePadding_ShadowDepthPass_LPV_44"
               OpMemberName %type_ShadowDepthPass 12 "PrePadding_ShadowDepthPass_LPV_48"
               OpMemberName %type_ShadowDepthPass 13 "PrePadding_ShadowDepthPass_LPV_52"
               OpMemberName %type_ShadowDepthPass 14 "PrePadding_ShadowDepthPass_LPV_56"
               OpMemberName %type_ShadowDepthPass 15 "PrePadding_ShadowDepthPass_LPV_60"
               OpMemberName %type_ShadowDepthPass 16 "PrePadding_ShadowDepthPass_LPV_64"
               OpMemberName %type_ShadowDepthPass 17 "PrePadding_ShadowDepthPass_LPV_68"
               OpMemberName %type_ShadowDepthPass 18 "PrePadding_ShadowDepthPass_LPV_72"
               OpMemberName %type_ShadowDepthPass 19 "PrePadding_ShadowDepthPass_LPV_76"
               OpMemberName %type_ShadowDepthPass 20 "PrePadding_ShadowDepthPass_LPV_80"
               OpMemberName %type_ShadowDepthPass 21 "PrePadding_ShadowDepthPass_LPV_84"
               OpMemberName %type_ShadowDepthPass 22 "PrePadding_ShadowDepthPass_LPV_88"
               OpMemberName %type_ShadowDepthPass 23 "PrePadding_ShadowDepthPass_LPV_92"
               OpMemberName %type_ShadowDepthPass 24 "PrePadding_ShadowDepthPass_LPV_96"
               OpMemberName %type_ShadowDepthPass 25 "PrePadding_ShadowDepthPass_LPV_100"
               OpMemberName %type_ShadowDepthPass 26 "PrePadding_ShadowDepthPass_LPV_104"
               OpMemberName %type_ShadowDepthPass 27 "PrePadding_ShadowDepthPass_LPV_108"
               OpMemberName %type_ShadowDepthPass 28 "PrePadding_ShadowDepthPass_LPV_112"
               OpMemberName %type_ShadowDepthPass 29 "PrePadding_ShadowDepthPass_LPV_116"
               OpMemberName %type_ShadowDepthPass 30 "PrePadding_ShadowDepthPass_LPV_120"
               OpMemberName %type_ShadowDepthPass 31 "PrePadding_ShadowDepthPass_LPV_124"
               OpMemberName %type_ShadowDepthPass 32 "PrePadding_ShadowDepthPass_LPV_128"
               OpMemberName %type_ShadowDepthPass 33 "PrePadding_ShadowDepthPass_LPV_132"
               OpMemberName %type_ShadowDepthPass 34 "PrePadding_ShadowDepthPass_LPV_136"
               OpMemberName %type_ShadowDepthPass 35 "PrePadding_ShadowDepthPass_LPV_140"
               OpMemberName %type_ShadowDepthPass 36 "PrePadding_ShadowDepthPass_LPV_144"
               OpMemberName %type_ShadowDepthPass 37 "PrePadding_ShadowDepthPass_LPV_148"
               OpMemberName %type_ShadowDepthPass 38 "PrePadding_ShadowDepthPass_LPV_152"
               OpMemberName %type_ShadowDepthPass 39 "PrePadding_ShadowDepthPass_LPV_156"
               OpMemberName %type_ShadowDepthPass 40 "PrePadding_ShadowDepthPass_LPV_160"
               OpMemberName %type_ShadowDepthPass 41 "PrePadding_ShadowDepthPass_LPV_164"
               OpMemberName %type_ShadowDepthPass 42 "PrePadding_ShadowDepthPass_LPV_168"
               OpMemberName %type_ShadowDepthPass 43 "PrePadding_ShadowDepthPass_LPV_172"
               OpMemberName %type_ShadowDepthPass 44 "PrePadding_ShadowDepthPass_LPV_176"
               OpMemberName %type_ShadowDepthPass 45 "PrePadding_ShadowDepthPass_LPV_180"
               OpMemberName %type_ShadowDepthPass 46 "PrePadding_ShadowDepthPass_LPV_184"
               OpMemberName %type_ShadowDepthPass 47 "PrePadding_ShadowDepthPass_LPV_188"
               OpMemberName %type_ShadowDepthPass 48 "PrePadding_ShadowDepthPass_LPV_192"
               OpMemberName %type_ShadowDepthPass 49 "PrePadding_ShadowDepthPass_LPV_196"
               OpMemberName %type_ShadowDepthPass 50 "PrePadding_ShadowDepthPass_LPV_200"
               OpMemberName %type_ShadowDepthPass 51 "PrePadding_ShadowDepthPass_LPV_204"
               OpMemberName %type_ShadowDepthPass 52 "PrePadding_ShadowDepthPass_LPV_208"
               OpMemberName %type_ShadowDepthPass 53 "PrePadding_ShadowDepthPass_LPV_212"
               OpMemberName %type_ShadowDepthPass 54 "PrePadding_ShadowDepthPass_LPV_216"
               OpMemberName %type_ShadowDepthPass 55 "PrePadding_ShadowDepthPass_LPV_220"
               OpMemberName %type_ShadowDepthPass 56 "PrePadding_ShadowDepthPass_LPV_224"
               OpMemberName %type_ShadowDepthPass 57 "PrePadding_ShadowDepthPass_LPV_228"
               OpMemberName %type_ShadowDepthPass 58 "PrePadding_ShadowDepthPass_LPV_232"
               OpMemberName %type_ShadowDepthPass 59 "PrePadding_ShadowDepthPass_LPV_236"
               OpMemberName %type_ShadowDepthPass 60 "PrePadding_ShadowDepthPass_LPV_240"
               OpMemberName %type_ShadowDepthPass 61 "PrePadding_ShadowDepthPass_LPV_244"
               OpMemberName %type_ShadowDepthPass 62 "PrePadding_ShadowDepthPass_LPV_248"
               OpMemberName %type_ShadowDepthPass 63 "PrePadding_ShadowDepthPass_LPV_252"
               OpMemberName %type_ShadowDepthPass 64 "PrePadding_ShadowDepthPass_LPV_256"
               OpMemberName %type_ShadowDepthPass 65 "PrePadding_ShadowDepthPass_LPV_260"
               OpMemberName %type_ShadowDepthPass 66 "PrePadding_ShadowDepthPass_LPV_264"
               OpMemberName %type_ShadowDepthPass 67 "PrePadding_ShadowDepthPass_LPV_268"
               OpMemberName %type_ShadowDepthPass 68 "ShadowDepthPass_LPV_mRsmToWorld"
               OpMemberName %type_ShadowDepthPass 69 "ShadowDepthPass_LPV_mLightColour"
               OpMemberName %type_ShadowDepthPass 70 "ShadowDepthPass_LPV_GeometryVolumeCaptureLightDirection"
               OpMemberName %type_ShadowDepthPass 71 "ShadowDepthPass_LPV_mEyePos"
               OpMemberName %type_ShadowDepthPass 72 "ShadowDepthPass_LPV_mOldGridOffset"
               OpMemberName %type_ShadowDepthPass 73 "PrePadding_ShadowDepthPass_LPV_396"
               OpMemberName %type_ShadowDepthPass 74 "ShadowDepthPass_LPV_mLpvGridOffset"
               OpMemberName %type_ShadowDepthPass 75 "ShadowDepthPass_LPV_ClearMultiplier"
               OpMemberName %type_ShadowDepthPass 76 "ShadowDepthPass_LPV_LpvScale"
               OpMemberName %type_ShadowDepthPass 77 "ShadowDepthPass_LPV_OneOverLpvScale"
               OpMemberName %type_ShadowDepthPass 78 "ShadowDepthPass_LPV_DirectionalOcclusionIntensity"
               OpMemberName %type_ShadowDepthPass 79 "ShadowDepthPass_LPV_DirectionalOcclusionRadius"
               OpMemberName %type_ShadowDepthPass 80 "ShadowDepthPass_LPV_RsmAreaIntensityMultiplier"
               OpMemberName %type_ShadowDepthPass 81 "ShadowDepthPass_LPV_RsmPixelToTexcoordMultiplier"
               OpMemberName %type_ShadowDepthPass 82 "ShadowDepthPass_LPV_SecondaryOcclusionStrength"
               OpMemberName %type_ShadowDepthPass 83 "ShadowDepthPass_LPV_SecondaryBounceStrength"
               OpMemberName %type_ShadowDepthPass 84 "ShadowDepthPass_LPV_VplInjectionBias"
               OpMemberName %type_ShadowDepthPass 85 "ShadowDepthPass_LPV_GeometryVolumeInjectionBias"
               OpMemberName %type_ShadowDepthPass 86 "ShadowDepthPass_LPV_EmissiveInjectionMultiplier"
               OpMemberName %type_ShadowDepthPass 87 "ShadowDepthPass_LPV_PropagationIndex"
               OpMemberName %type_ShadowDepthPass 88 "ShadowDepthPass_ProjectionMatrix"
               OpMemberName %type_ShadowDepthPass 89 "ShadowDepthPass_ViewMatrix"
               OpMemberName %type_ShadowDepthPass 90 "ShadowDepthPass_ShadowParams"
               OpMemberName %type_ShadowDepthPass 91 "ShadowDepthPass_bClampToNearPlane"
               OpMemberName %type_ShadowDepthPass 92 "PrePadding_ShadowDepthPass_612"
               OpMemberName %type_ShadowDepthPass 93 "PrePadding_ShadowDepthPass_616"
               OpMemberName %type_ShadowDepthPass 94 "PrePadding_ShadowDepthPass_620"
               OpMemberName %type_ShadowDepthPass 95 "ShadowDepthPass_ShadowViewProjectionMatrices"
               OpMemberName %type_ShadowDepthPass 96 "ShadowDepthPass_ShadowViewMatrices"
               OpName %ShadowDepthPass "ShadowDepthPass"
               OpName %in_var_TEXCOORD10_centroid "in.var.TEXCOORD10_centroid"
               OpName %in_var_TEXCOORD11_centroid "in.var.TEXCOORD11_centroid"
               OpName %in_var_VS_to_DS_Position "in.var.VS_to_DS_Position"
               OpName %in_var_VS_To_DS_VertexID "in.var.VS_To_DS_VertexID"
               OpName %in_var_PN_POSITION "in.var.PN_POSITION"
               OpName %in_var_PN_DisplacementScales "in.var.PN_DisplacementScales"
               OpName %in_var_PN_TessellationMultiplier "in.var.PN_TessellationMultiplier"
               OpName %in_var_PN_WorldDisplacementMultiplier "in.var.PN_WorldDisplacementMultiplier"
               OpName %in_var_PN_DominantVertex "in.var.PN_DominantVertex"
               OpName %in_var_PN_DominantVertex1 "in.var.PN_DominantVertex1"
               OpName %in_var_PN_DominantVertex2 "in.var.PN_DominantVertex2"
               OpName %in_var_PN_DominantEdge "in.var.PN_DominantEdge"
               OpName %in_var_PN_DominantEdge1 "in.var.PN_DominantEdge1"
               OpName %in_var_PN_DominantEdge2 "in.var.PN_DominantEdge2"
               OpName %in_var_PN_DominantEdge3 "in.var.PN_DominantEdge3"
               OpName %in_var_PN_DominantEdge4 "in.var.PN_DominantEdge4"
               OpName %in_var_PN_DominantEdge5 "in.var.PN_DominantEdge5"
               OpName %in_var_PN_POSITION9 "in.var.PN_POSITION9"
               OpName %out_var_TEXCOORD10_centroid "out.var.TEXCOORD10_centroid"
               OpName %out_var_TEXCOORD11_centroid "out.var.TEXCOORD11_centroid"
               OpName %out_var_TEXCOORD6 "out.var.TEXCOORD6"
               OpName %out_var_TEXCOORD7 "out.var.TEXCOORD7"
               OpName %MainDomain "MainDomain"
               OpDecorateString %in_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %in_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %in_var_VS_to_DS_Position UserSemantic "VS_to_DS_Position"
               OpDecorateString %in_var_VS_To_DS_VertexID UserSemantic "VS_To_DS_VertexID"
               OpDecorateString %in_var_PN_POSITION UserSemantic "PN_POSITION"
               OpDecorateString %in_var_PN_DisplacementScales UserSemantic "PN_DisplacementScales"
               OpDecorateString %in_var_PN_TessellationMultiplier UserSemantic "PN_TessellationMultiplier"
               OpDecorateString %in_var_PN_WorldDisplacementMultiplier UserSemantic "PN_WorldDisplacementMultiplier"
               OpDecorateString %in_var_PN_DominantVertex UserSemantic "PN_DominantVertex"
               OpDecorateString %in_var_PN_DominantVertex1 UserSemantic "PN_DominantVertex"
               OpDecorateString %in_var_PN_DominantVertex2 UserSemantic "PN_DominantVertex"
               OpDecorateString %in_var_PN_DominantEdge UserSemantic "PN_DominantEdge"
               OpDecorateString %in_var_PN_DominantEdge1 UserSemantic "PN_DominantEdge"
               OpDecorateString %in_var_PN_DominantEdge2 UserSemantic "PN_DominantEdge"
               OpDecorateString %in_var_PN_DominantEdge3 UserSemantic "PN_DominantEdge"
               OpDecorateString %in_var_PN_DominantEdge4 UserSemantic "PN_DominantEdge"
               OpDecorateString %in_var_PN_DominantEdge5 UserSemantic "PN_DominantEdge"
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorateString %gl_TessLevelOuter UserSemantic "SV_TessFactor"
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorateString %gl_TessLevelInner UserSemantic "SV_InsideTessFactor"
               OpDecorate %gl_TessLevelInner Patch
               OpDecorateString %in_var_PN_POSITION9 UserSemantic "PN_POSITION9"
               OpDecorate %in_var_PN_POSITION9 Patch
               OpDecorate %gl_TessCoord BuiltIn TessCoord
               OpDecorateString %gl_TessCoord UserSemantic "SV_DomainLocation"
               OpDecorate %gl_TessCoord Patch
               OpDecorateString %out_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %out_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %out_var_TEXCOORD6 UserSemantic "TEXCOORD6"
               OpDecorateString %out_var_TEXCOORD7 UserSemantic "TEXCOORD7"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_POSITION"
               OpDecorate %in_var_PN_DisplacementScales Location 0
               OpDecorate %in_var_PN_DominantEdge Location 1
               OpDecorate %in_var_PN_DominantEdge1 Location 2
               OpDecorate %in_var_PN_DominantEdge2 Location 3
               OpDecorate %in_var_PN_DominantEdge3 Location 4
               OpDecorate %in_var_PN_DominantEdge4 Location 5
               OpDecorate %in_var_PN_DominantEdge5 Location 6
               OpDecorate %in_var_PN_DominantVertex Location 7
               OpDecorate %in_var_PN_DominantVertex1 Location 8
               OpDecorate %in_var_PN_DominantVertex2 Location 9
               OpDecorate %in_var_PN_POSITION Location 10
               OpDecorate %in_var_PN_POSITION9 Location 13
               OpDecorate %in_var_PN_TessellationMultiplier Location 14
               OpDecorate %in_var_PN_WorldDisplacementMultiplier Location 15
               OpDecorate %in_var_TEXCOORD10_centroid Location 16
               OpDecorate %in_var_TEXCOORD11_centroid Location 17
               OpDecorate %in_var_VS_To_DS_VertexID Location 18
               OpDecorate %in_var_VS_to_DS_Position Location 19
               OpDecorate %out_var_TEXCOORD10_centroid Location 0
               OpDecorate %out_var_TEXCOORD11_centroid Location 1
               OpDecorate %out_var_TEXCOORD6 Location 2
               OpDecorate %out_var_TEXCOORD7 Location 3
               OpDecorate %ShadowDepthPass DescriptorSet 0
               OpDecorate %ShadowDepthPass Binding 0
               OpDecorate %_arr_mat4v4float_uint_6 ArrayStride 64
               OpMemberDecorate %type_ShadowDepthPass 0 Offset 0
               OpMemberDecorate %type_ShadowDepthPass 1 Offset 4
               OpMemberDecorate %type_ShadowDepthPass 2 Offset 8
               OpMemberDecorate %type_ShadowDepthPass 3 Offset 12
               OpMemberDecorate %type_ShadowDepthPass 4 Offset 16
               OpMemberDecorate %type_ShadowDepthPass 5 Offset 20
               OpMemberDecorate %type_ShadowDepthPass 6 Offset 24
               OpMemberDecorate %type_ShadowDepthPass 7 Offset 28
               OpMemberDecorate %type_ShadowDepthPass 8 Offset 32
               OpMemberDecorate %type_ShadowDepthPass 9 Offset 36
               OpMemberDecorate %type_ShadowDepthPass 10 Offset 40
               OpMemberDecorate %type_ShadowDepthPass 11 Offset 44
               OpMemberDecorate %type_ShadowDepthPass 12 Offset 48
               OpMemberDecorate %type_ShadowDepthPass 13 Offset 52
               OpMemberDecorate %type_ShadowDepthPass 14 Offset 56
               OpMemberDecorate %type_ShadowDepthPass 15 Offset 60
               OpMemberDecorate %type_ShadowDepthPass 16 Offset 64
               OpMemberDecorate %type_ShadowDepthPass 17 Offset 68
               OpMemberDecorate %type_ShadowDepthPass 18 Offset 72
               OpMemberDecorate %type_ShadowDepthPass 19 Offset 76
               OpMemberDecorate %type_ShadowDepthPass 20 Offset 80
               OpMemberDecorate %type_ShadowDepthPass 21 Offset 84
               OpMemberDecorate %type_ShadowDepthPass 22 Offset 88
               OpMemberDecorate %type_ShadowDepthPass 23 Offset 92
               OpMemberDecorate %type_ShadowDepthPass 24 Offset 96
               OpMemberDecorate %type_ShadowDepthPass 25 Offset 100
               OpMemberDecorate %type_ShadowDepthPass 26 Offset 104
               OpMemberDecorate %type_ShadowDepthPass 27 Offset 108
               OpMemberDecorate %type_ShadowDepthPass 28 Offset 112
               OpMemberDecorate %type_ShadowDepthPass 29 Offset 116
               OpMemberDecorate %type_ShadowDepthPass 30 Offset 120
               OpMemberDecorate %type_ShadowDepthPass 31 Offset 124
               OpMemberDecorate %type_ShadowDepthPass 32 Offset 128
               OpMemberDecorate %type_ShadowDepthPass 33 Offset 132
               OpMemberDecorate %type_ShadowDepthPass 34 Offset 136
               OpMemberDecorate %type_ShadowDepthPass 35 Offset 140
               OpMemberDecorate %type_ShadowDepthPass 36 Offset 144
               OpMemberDecorate %type_ShadowDepthPass 37 Offset 148
               OpMemberDecorate %type_ShadowDepthPass 38 Offset 152
               OpMemberDecorate %type_ShadowDepthPass 39 Offset 156
               OpMemberDecorate %type_ShadowDepthPass 40 Offset 160
               OpMemberDecorate %type_ShadowDepthPass 41 Offset 164
               OpMemberDecorate %type_ShadowDepthPass 42 Offset 168
               OpMemberDecorate %type_ShadowDepthPass 43 Offset 172
               OpMemberDecorate %type_ShadowDepthPass 44 Offset 176
               OpMemberDecorate %type_ShadowDepthPass 45 Offset 180
               OpMemberDecorate %type_ShadowDepthPass 46 Offset 184
               OpMemberDecorate %type_ShadowDepthPass 47 Offset 188
               OpMemberDecorate %type_ShadowDepthPass 48 Offset 192
               OpMemberDecorate %type_ShadowDepthPass 49 Offset 196
               OpMemberDecorate %type_ShadowDepthPass 50 Offset 200
               OpMemberDecorate %type_ShadowDepthPass 51 Offset 204
               OpMemberDecorate %type_ShadowDepthPass 52 Offset 208
               OpMemberDecorate %type_ShadowDepthPass 53 Offset 212
               OpMemberDecorate %type_ShadowDepthPass 54 Offset 216
               OpMemberDecorate %type_ShadowDepthPass 55 Offset 220
               OpMemberDecorate %type_ShadowDepthPass 56 Offset 224
               OpMemberDecorate %type_ShadowDepthPass 57 Offset 228
               OpMemberDecorate %type_ShadowDepthPass 58 Offset 232
               OpMemberDecorate %type_ShadowDepthPass 59 Offset 236
               OpMemberDecorate %type_ShadowDepthPass 60 Offset 240
               OpMemberDecorate %type_ShadowDepthPass 61 Offset 244
               OpMemberDecorate %type_ShadowDepthPass 62 Offset 248
               OpMemberDecorate %type_ShadowDepthPass 63 Offset 252
               OpMemberDecorate %type_ShadowDepthPass 64 Offset 256
               OpMemberDecorate %type_ShadowDepthPass 65 Offset 260
               OpMemberDecorate %type_ShadowDepthPass 66 Offset 264
               OpMemberDecorate %type_ShadowDepthPass 67 Offset 268
               OpMemberDecorate %type_ShadowDepthPass 68 Offset 272
               OpMemberDecorate %type_ShadowDepthPass 68 MatrixStride 16
               OpMemberDecorate %type_ShadowDepthPass 68 ColMajor
               OpMemberDecorate %type_ShadowDepthPass 69 Offset 336
               OpMemberDecorate %type_ShadowDepthPass 70 Offset 352
               OpMemberDecorate %type_ShadowDepthPass 71 Offset 368
               OpMemberDecorate %type_ShadowDepthPass 72 Offset 384
               OpMemberDecorate %type_ShadowDepthPass 73 Offset 396
               OpMemberDecorate %type_ShadowDepthPass 74 Offset 400
               OpMemberDecorate %type_ShadowDepthPass 75 Offset 412
               OpMemberDecorate %type_ShadowDepthPass 76 Offset 416
               OpMemberDecorate %type_ShadowDepthPass 77 Offset 420
               OpMemberDecorate %type_ShadowDepthPass 78 Offset 424
               OpMemberDecorate %type_ShadowDepthPass 79 Offset 428
               OpMemberDecorate %type_ShadowDepthPass 80 Offset 432
               OpMemberDecorate %type_ShadowDepthPass 81 Offset 436
               OpMemberDecorate %type_ShadowDepthPass 82 Offset 440
               OpMemberDecorate %type_ShadowDepthPass 83 Offset 444
               OpMemberDecorate %type_ShadowDepthPass 84 Offset 448
               OpMemberDecorate %type_ShadowDepthPass 85 Offset 452
               OpMemberDecorate %type_ShadowDepthPass 86 Offset 456
               OpMemberDecorate %type_ShadowDepthPass 87 Offset 460
               OpMemberDecorate %type_ShadowDepthPass 88 Offset 464
               OpMemberDecorate %type_ShadowDepthPass 88 MatrixStride 16
               OpMemberDecorate %type_ShadowDepthPass 88 ColMajor
               OpMemberDecorate %type_ShadowDepthPass 89 Offset 528
               OpMemberDecorate %type_ShadowDepthPass 89 MatrixStride 16
               OpMemberDecorate %type_ShadowDepthPass 89 ColMajor
               OpMemberDecorate %type_ShadowDepthPass 90 Offset 592
               OpMemberDecorate %type_ShadowDepthPass 91 Offset 608
               OpMemberDecorate %type_ShadowDepthPass 92 Offset 612
               OpMemberDecorate %type_ShadowDepthPass 93 Offset 616
               OpMemberDecorate %type_ShadowDepthPass 94 Offset 620
               OpMemberDecorate %type_ShadowDepthPass 95 Offset 624
               OpMemberDecorate %type_ShadowDepthPass 95 MatrixStride 16
               OpMemberDecorate %type_ShadowDepthPass 95 ColMajor
               OpMemberDecorate %type_ShadowDepthPass 96 Offset 1008
               OpMemberDecorate %type_ShadowDepthPass 96 MatrixStride 16
               OpMemberDecorate %type_ShadowDepthPass 96 ColMajor
               OpDecorate %type_ShadowDepthPass Block
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_4 = OpConstant %uint 4
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
    %float_3 = OpConstant %float 3
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %float_6 = OpConstant %float 6
         %48 = OpConstantComposite %v4float %float_6 %float_6 %float_6 %float_6
    %float_1 = OpConstant %float 1
    %float_0 = OpConstant %float 0
      %int_3 = OpConstant %int 3
     %int_88 = OpConstant %int 88
     %int_89 = OpConstant %int 89
     %int_90 = OpConstant %int 90
     %int_91 = OpConstant %int 91
%float_9_99999997en07 = OpConstant %float 9.99999997e-07
     %uint_6 = OpConstant %uint 6
%_arr_mat4v4float_uint_6 = OpTypeArray %mat4v4float %uint_6
      %v3int = OpTypeVector %int 3
%type_ShadowDepthPass = OpTypeStruct %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %mat4v4float %v4float %v4float %v4float %v3int %int %v3int %float %float %float %float %float %float %float %float %float %float %float %float %int %mat4v4float %mat4v4float %v4float %float %float %float %float %_arr_mat4v4float_uint_6 %_arr_mat4v4float_uint_6
%_ptr_Uniform_type_ShadowDepthPass = OpTypePointer Uniform %type_ShadowDepthPass
     %uint_3 = OpConstant %uint 3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%_ptr_Input__arr_v4float_uint_3 = OpTypePointer Input %_arr_v4float_uint_3
%_arr_uint_uint_3 = OpTypeArray %uint %uint_3
%_ptr_Input__arr_uint_uint_3 = OpTypePointer Input %_arr_uint_uint_3
%_arr__arr_v4float_uint_3_uint_3 = OpTypeArray %_arr_v4float_uint_3 %uint_3
%_ptr_Input__arr__arr_v4float_uint_3_uint_3 = OpTypePointer Input %_arr__arr_v4float_uint_3_uint_3
%_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
%_ptr_Input__arr_v3float_uint_3 = OpTypePointer Input %_arr_v3float_uint_3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
%_ptr_Input__arr_float_uint_3 = OpTypePointer Input %_arr_float_uint_3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
%_ptr_Input__arr_v2float_uint_3 = OpTypePointer Input %_arr_v2float_uint_3
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Input__arr_float_uint_4 = OpTypePointer Input %_arr_float_uint_4
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Input__arr_float_uint_2 = OpTypePointer Input %_arr_float_uint_2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_v3float = OpTypePointer Input %v3float
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Output_v3float = OpTypePointer Output %v3float
       %void = OpTypeVoid
         %83 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
       %bool = OpTypeBool
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%ShadowDepthPass = OpVariable %_ptr_Uniform_type_ShadowDepthPass Uniform
%in_var_TEXCOORD10_centroid = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%in_var_TEXCOORD11_centroid = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%in_var_VS_to_DS_Position = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%in_var_VS_To_DS_VertexID = OpVariable %_ptr_Input__arr_uint_uint_3 Input
%in_var_PN_POSITION = OpVariable %_ptr_Input__arr__arr_v4float_uint_3_uint_3 Input
%in_var_PN_DisplacementScales = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
%in_var_PN_TessellationMultiplier = OpVariable %_ptr_Input__arr_float_uint_3 Input
%in_var_PN_WorldDisplacementMultiplier = OpVariable %_ptr_Input__arr_float_uint_3 Input
%in_var_PN_DominantVertex = OpVariable %_ptr_Input__arr_v2float_uint_3 Input
%in_var_PN_DominantVertex1 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%in_var_PN_DominantVertex2 = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
%in_var_PN_DominantEdge = OpVariable %_ptr_Input__arr_v2float_uint_3 Input
%in_var_PN_DominantEdge1 = OpVariable %_ptr_Input__arr_v2float_uint_3 Input
%in_var_PN_DominantEdge2 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%in_var_PN_DominantEdge3 = OpVariable %_ptr_Input__arr_v4float_uint_3 Input
%in_var_PN_DominantEdge4 = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
%in_var_PN_DominantEdge5 = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
%gl_TessLevelOuter = OpVariable %_ptr_Input__arr_float_uint_4 Input
%gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input
%in_var_PN_POSITION9 = OpVariable %_ptr_Input_v4float Input
%gl_TessCoord = OpVariable %_ptr_Input_v3float Input
%out_var_TEXCOORD10_centroid = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD11_centroid = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD6 = OpVariable %_ptr_Output_float Output
%out_var_TEXCOORD7 = OpVariable %_ptr_Output_v3float Output
%gl_Position = OpVariable %_ptr_Output_v4float Output
         %89 = OpConstantNull %v4float
         %90 = OpUndef %v4float
 %MainDomain = OpFunction %void None %83
         %91 = OpLabel
         %92 = OpVariable %_ptr_Function_mat4v4float Function
         %93 = OpLoad %_arr_v4float_uint_3 %in_var_TEXCOORD10_centroid
         %94 = OpLoad %_arr_v4float_uint_3 %in_var_TEXCOORD11_centroid
         %95 = OpCompositeExtract %v4float %93 0
         %96 = OpCompositeExtract %v4float %94 0
         %97 = OpCompositeExtract %v4float %93 1
         %98 = OpCompositeExtract %v4float %94 1
         %99 = OpCompositeExtract %v4float %93 2
        %100 = OpCompositeExtract %v4float %94 2
        %101 = OpLoad %_arr__arr_v4float_uint_3_uint_3 %in_var_PN_POSITION
        %102 = OpCompositeExtract %_arr_v4float_uint_3 %101 0
        %103 = OpCompositeExtract %_arr_v4float_uint_3 %101 1
        %104 = OpCompositeExtract %_arr_v4float_uint_3 %101 2
        %105 = OpCompositeExtract %v4float %102 0
        %106 = OpCompositeExtract %v4float %102 1
        %107 = OpCompositeExtract %v4float %102 2
        %108 = OpCompositeExtract %v4float %103 0
        %109 = OpCompositeExtract %v4float %103 1
        %110 = OpCompositeExtract %v4float %103 2
        %111 = OpCompositeExtract %v4float %104 0
        %112 = OpCompositeExtract %v4float %104 1
        %113 = OpCompositeExtract %v4float %104 2
        %114 = OpLoad %v4float %in_var_PN_POSITION9
        %115 = OpLoad %v3float %gl_TessCoord
        %116 = OpCompositeExtract %float %115 0
        %117 = OpCompositeExtract %float %115 1
        %118 = OpCompositeExtract %float %115 2
        %119 = OpFMul %float %116 %116
        %120 = OpFMul %float %117 %117
        %121 = OpFMul %float %118 %118
        %122 = OpFMul %float %119 %float_3
        %123 = OpFMul %float %120 %float_3
        %124 = OpFMul %float %121 %float_3
        %125 = OpCompositeConstruct %v4float %119 %119 %119 %119
        %126 = OpFMul %v4float %105 %125
        %127 = OpCompositeConstruct %v4float %116 %116 %116 %116
        %128 = OpFMul %v4float %126 %127
        %129 = OpCompositeConstruct %v4float %120 %120 %120 %120
        %130 = OpFMul %v4float %108 %129
        %131 = OpCompositeConstruct %v4float %117 %117 %117 %117
        %132 = OpFMul %v4float %130 %131
        %133 = OpFAdd %v4float %128 %132
        %134 = OpCompositeConstruct %v4float %121 %121 %121 %121
        %135 = OpFMul %v4float %111 %134
        %136 = OpCompositeConstruct %v4float %118 %118 %118 %118
        %137 = OpFMul %v4float %135 %136
        %138 = OpFAdd %v4float %133 %137
        %139 = OpCompositeConstruct %v4float %122 %122 %122 %122
        %140 = OpFMul %v4float %106 %139
        %141 = OpFMul %v4float %140 %131
        %142 = OpFAdd %v4float %138 %141
        %143 = OpCompositeConstruct %v4float %123 %123 %123 %123
        %144 = OpFMul %v4float %107 %143
        %145 = OpFMul %v4float %144 %127
        %146 = OpFAdd %v4float %142 %145
        %147 = OpFMul %v4float %109 %143
        %148 = OpFMul %v4float %147 %136
        %149 = OpFAdd %v4float %146 %148
        %150 = OpCompositeConstruct %v4float %124 %124 %124 %124
        %151 = OpFMul %v4float %110 %150
        %152 = OpFMul %v4float %151 %131
        %153 = OpFAdd %v4float %149 %152
        %154 = OpFMul %v4float %112 %150
        %155 = OpFMul %v4float %154 %127
        %156 = OpFAdd %v4float %153 %155
        %157 = OpFMul %v4float %113 %139
        %158 = OpFMul %v4float %157 %136
        %159 = OpFAdd %v4float %156 %158
        %160 = OpFMul %v4float %114 %48
        %161 = OpFMul %v4float %160 %136
        %162 = OpFMul %v4float %161 %127
        %163 = OpFMul %v4float %162 %131
        %164 = OpFAdd %v4float %159 %163
        %165 = OpVectorShuffle %v3float %95 %95 0 1 2
        %166 = OpCompositeConstruct %v3float %116 %116 %116
        %167 = OpFMul %v3float %165 %166
        %168 = OpVectorShuffle %v3float %97 %97 0 1 2
        %169 = OpCompositeConstruct %v3float %117 %117 %117
        %170 = OpFMul %v3float %168 %169
        %171 = OpFAdd %v3float %167 %170
        %172 = OpFMul %v4float %96 %127
        %173 = OpFMul %v4float %98 %131
        %174 = OpFAdd %v4float %172 %173
        %175 = OpVectorShuffle %v3float %171 %89 0 1 2
        %176 = OpVectorShuffle %v3float %99 %99 0 1 2
        %177 = OpCompositeConstruct %v3float %118 %118 %118
        %178 = OpFMul %v3float %176 %177
        %179 = OpFAdd %v3float %175 %178
        %180 = OpVectorShuffle %v4float %90 %179 4 5 6 3
        %181 = OpFMul %v4float %100 %136
        %182 = OpFAdd %v4float %174 %181
        %183 = OpVectorShuffle %v3float %182 %182 0 1 2
        %184 = OpVectorShuffle %v4float %164 %164 4 5 6 3
        %185 = OpAccessChain %_ptr_Uniform_mat4v4float %ShadowDepthPass %int_88
        %186 = OpLoad %mat4v4float %185
        %187 = OpAccessChain %_ptr_Uniform_mat4v4float %ShadowDepthPass %int_89
        %188 = OpLoad %mat4v4float %187
               OpStore %92 %188
        %189 = OpMatrixTimesVector %v4float %186 %184
        %190 = OpAccessChain %_ptr_Uniform_float %ShadowDepthPass %int_91
        %191 = OpLoad %float %190
        %192 = OpFOrdGreaterThan %bool %191 %float_0
        %193 = OpCompositeExtract %float %189 2
        %194 = OpFOrdLessThan %bool %193 %float_0
        %195 = OpLogicalAnd %bool %192 %194
               OpSelectionMerge %196 None
               OpBranchConditional %195 %197 %196
        %197 = OpLabel
        %198 = OpCompositeInsert %v4float %float_9_99999997en07 %189 2
        %199 = OpCompositeInsert %v4float %float_1 %198 3
               OpBranch %196
        %196 = OpLabel
        %200 = OpPhi %v4float %189 %91 %199 %197
        %201 = OpAccessChain %_ptr_Function_float %92 %uint_0 %int_2
        %202 = OpLoad %float %201
        %203 = OpAccessChain %_ptr_Function_float %92 %uint_1 %int_2
        %204 = OpLoad %float %203
        %205 = OpAccessChain %_ptr_Function_float %92 %uint_2 %int_2
        %206 = OpLoad %float %205
        %207 = OpCompositeConstruct %v3float %202 %204 %206
        %208 = OpDot %float %207 %183
        %209 = OpExtInst %float %1 FAbs %208
        %210 = OpAccessChain %_ptr_Uniform_float %ShadowDepthPass %int_90 %int_2
        %211 = OpLoad %float %210
        %212 = OpExtInst %float %1 FAbs %209
        %213 = OpFOrdGreaterThan %bool %212 %float_0
        %214 = OpFMul %float %209 %209
        %215 = OpFSub %float %float_1 %214
        %216 = OpExtInst %float %1 FClamp %215 %float_0 %float_1
        %217 = OpExtInst %float %1 Sqrt %216
        %218 = OpFDiv %float %217 %209
        %219 = OpSelect %float %213 %218 %211
        %220 = OpExtInst %float %1 FClamp %219 %float_0 %211
        %221 = OpAccessChain %_ptr_Uniform_float %ShadowDepthPass %int_90 %int_1
        %222 = OpLoad %float %221
        %223 = OpFMul %float %222 %220
        %224 = OpAccessChain %_ptr_Uniform_float %ShadowDepthPass %int_90 %int_0
        %225 = OpLoad %float %224
        %226 = OpFAdd %float %223 %225
        %227 = OpAccessChain %_ptr_Uniform_float %ShadowDepthPass %int_90 %int_3
        %228 = OpLoad %float %227
        %229 = OpCompositeExtract %float %200 2
        %230 = OpFMul %float %229 %228
        %231 = OpFAdd %float %230 %226
        %232 = OpCompositeExtract %float %200 3
        %233 = OpFMul %float %231 %232
        %234 = OpCompositeInsert %v4float %233 %200 2
        %235 = OpVectorShuffle %v3float %164 %89 0 1 2
               OpStore %out_var_TEXCOORD10_centroid %180
               OpStore %out_var_TEXCOORD11_centroid %182
               OpStore %out_var_TEXCOORD6 %float_0
               OpStore %out_var_TEXCOORD7 %235
               OpStore %gl_Position %234
               OpReturn
               OpFunctionEnd
