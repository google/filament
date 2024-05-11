; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 598
; Schema: 0
               OpCapability Tessellation
               OpCapability SampledBuffer
               OpCapability StorageImageExtendedFormats
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %MainHull "main" %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_COLOR0 %in_var_TEXCOORD0 %in_var_TEXCOORD4 %in_var_PRIMITIVE_ID %in_var_LIGHTMAP_ID %in_var_VS_To_DS_Position %gl_InvocationID %out_var_TEXCOORD10_centroid %out_var_TEXCOORD11_centroid %out_var_COLOR0 %out_var_TEXCOORD0 %out_var_TEXCOORD4 %out_var_PRIMITIVE_ID %out_var_LIGHTMAP_ID %out_var_VS_To_DS_Position %out_var_PN_POSITION %out_var_PN_DisplacementScales %out_var_PN_TessellationMultiplier %out_var_PN_WorldDisplacementMultiplier %gl_TessLevelOuter %gl_TessLevelInner %out_var_PN_POSITION9
               OpExecutionMode %MainHull Triangles
               OpExecutionMode %MainHull SpacingFractionalOdd
               OpExecutionMode %MainHull VertexOrderCw
               OpExecutionMode %MainHull OutputVertices 3
               OpSource HLSL 600
               OpName %FPNTessellationHSToDS "FPNTessellationHSToDS"
               OpMemberName %FPNTessellationHSToDS 0 "PassSpecificData"
               OpMemberName %FPNTessellationHSToDS 1 "WorldPosition"
               OpMemberName %FPNTessellationHSToDS 2 "DisplacementScale"
               OpMemberName %FPNTessellationHSToDS 3 "TessellationMultiplier"
               OpMemberName %FPNTessellationHSToDS 4 "WorldDisplacementMultiplier"
               OpName %FBasePassVSToDS "FBasePassVSToDS"
               OpMemberName %FBasePassVSToDS 0 "FactoryInterpolants"
               OpMemberName %FBasePassVSToDS 1 "BasePassInterpolants"
               OpMemberName %FBasePassVSToDS 2 "Position"
               OpName %FVertexFactoryInterpolantsVSToDS "FVertexFactoryInterpolantsVSToDS"
               OpMemberName %FVertexFactoryInterpolantsVSToDS 0 "InterpolantsVSToPS"
               OpName %FVertexFactoryInterpolantsVSToPS "FVertexFactoryInterpolantsVSToPS"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 0 "TangentToWorld0"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 1 "TangentToWorld2"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 2 "Color"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 3 "TexCoords"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 4 "LightMapCoordinate"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 5 "PrimitiveId"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 6 "LightmapDataIndex"
               OpName %FBasePassInterpolantsVSToDS "FBasePassInterpolantsVSToDS"
               OpName %FSharedBasePassInterpolants "FSharedBasePassInterpolants"
               OpName %type_View "type.View"
               OpMemberName %type_View 0 "View_TranslatedWorldToClip"
               OpMemberName %type_View 1 "View_WorldToClip"
               OpMemberName %type_View 2 "View_TranslatedWorldToView"
               OpMemberName %type_View 3 "View_ViewToTranslatedWorld"
               OpMemberName %type_View 4 "View_TranslatedWorldToCameraView"
               OpMemberName %type_View 5 "View_CameraViewToTranslatedWorld"
               OpMemberName %type_View 6 "View_ViewToClip"
               OpMemberName %type_View 7 "View_ViewToClipNoAA"
               OpMemberName %type_View 8 "View_ClipToView"
               OpMemberName %type_View 9 "View_ClipToTranslatedWorld"
               OpMemberName %type_View 10 "View_SVPositionToTranslatedWorld"
               OpMemberName %type_View 11 "View_ScreenToWorld"
               OpMemberName %type_View 12 "View_ScreenToTranslatedWorld"
               OpMemberName %type_View 13 "View_ViewForward"
               OpMemberName %type_View 14 "PrePadding_View_844"
               OpMemberName %type_View 15 "View_ViewUp"
               OpMemberName %type_View 16 "PrePadding_View_860"
               OpMemberName %type_View 17 "View_ViewRight"
               OpMemberName %type_View 18 "PrePadding_View_876"
               OpMemberName %type_View 19 "View_HMDViewNoRollUp"
               OpMemberName %type_View 20 "PrePadding_View_892"
               OpMemberName %type_View 21 "View_HMDViewNoRollRight"
               OpMemberName %type_View 22 "PrePadding_View_908"
               OpMemberName %type_View 23 "View_InvDeviceZToWorldZTransform"
               OpMemberName %type_View 24 "View_ScreenPositionScaleBias"
               OpMemberName %type_View 25 "View_WorldCameraOrigin"
               OpMemberName %type_View 26 "PrePadding_View_956"
               OpMemberName %type_View 27 "View_TranslatedWorldCameraOrigin"
               OpMemberName %type_View 28 "PrePadding_View_972"
               OpMemberName %type_View 29 "View_WorldViewOrigin"
               OpMemberName %type_View 30 "PrePadding_View_988"
               OpMemberName %type_View 31 "View_PreViewTranslation"
               OpMemberName %type_View 32 "PrePadding_View_1004"
               OpMemberName %type_View 33 "View_PrevProjection"
               OpMemberName %type_View 34 "View_PrevViewProj"
               OpMemberName %type_View 35 "View_PrevViewRotationProj"
               OpMemberName %type_View 36 "View_PrevViewToClip"
               OpMemberName %type_View 37 "View_PrevClipToView"
               OpMemberName %type_View 38 "View_PrevTranslatedWorldToClip"
               OpMemberName %type_View 39 "View_PrevTranslatedWorldToView"
               OpMemberName %type_View 40 "View_PrevViewToTranslatedWorld"
               OpMemberName %type_View 41 "View_PrevTranslatedWorldToCameraView"
               OpMemberName %type_View 42 "View_PrevCameraViewToTranslatedWorld"
               OpMemberName %type_View 43 "View_PrevWorldCameraOrigin"
               OpMemberName %type_View 44 "PrePadding_View_1660"
               OpMemberName %type_View 45 "View_PrevWorldViewOrigin"
               OpMemberName %type_View 46 "PrePadding_View_1676"
               OpMemberName %type_View 47 "View_PrevPreViewTranslation"
               OpMemberName %type_View 48 "PrePadding_View_1692"
               OpMemberName %type_View 49 "View_PrevInvViewProj"
               OpMemberName %type_View 50 "View_PrevScreenToTranslatedWorld"
               OpMemberName %type_View 51 "View_ClipToPrevClip"
               OpMemberName %type_View 52 "View_TemporalAAJitter"
               OpMemberName %type_View 53 "View_GlobalClippingPlane"
               OpMemberName %type_View 54 "View_FieldOfViewWideAngles"
               OpMemberName %type_View 55 "View_PrevFieldOfViewWideAngles"
               OpMemberName %type_View 56 "View_ViewRectMin"
               OpMemberName %type_View 57 "View_ViewSizeAndInvSize"
               OpMemberName %type_View 58 "View_BufferSizeAndInvSize"
               OpMemberName %type_View 59 "View_BufferBilinearUVMinMax"
               OpMemberName %type_View 60 "View_NumSceneColorMSAASamples"
               OpMemberName %type_View 61 "View_PreExposure"
               OpMemberName %type_View 62 "View_OneOverPreExposure"
               OpMemberName %type_View 63 "PrePadding_View_2012"
               OpMemberName %type_View 64 "View_DiffuseOverrideParameter"
               OpMemberName %type_View 65 "View_SpecularOverrideParameter"
               OpMemberName %type_View 66 "View_NormalOverrideParameter"
               OpMemberName %type_View 67 "View_RoughnessOverrideParameter"
               OpMemberName %type_View 68 "View_PrevFrameGameTime"
               OpMemberName %type_View 69 "View_PrevFrameRealTime"
               OpMemberName %type_View 70 "View_OutOfBoundsMask"
               OpMemberName %type_View 71 "PrePadding_View_2084"
               OpMemberName %type_View 72 "PrePadding_View_2088"
               OpMemberName %type_View 73 "PrePadding_View_2092"
               OpMemberName %type_View 74 "View_WorldCameraMovementSinceLastFrame"
               OpMemberName %type_View 75 "View_CullingSign"
               OpMemberName %type_View 76 "View_NearPlane"
               OpMemberName %type_View 77 "View_AdaptiveTessellationFactor"
               OpMemberName %type_View 78 "View_GameTime"
               OpMemberName %type_View 79 "View_RealTime"
               OpMemberName %type_View 80 "View_DeltaTime"
               OpMemberName %type_View 81 "View_MaterialTextureMipBias"
               OpMemberName %type_View 82 "View_MaterialTextureDerivativeMultiply"
               OpMemberName %type_View 83 "View_Random"
               OpMemberName %type_View 84 "View_FrameNumber"
               OpMemberName %type_View 85 "View_StateFrameIndexMod8"
               OpMemberName %type_View 86 "View_StateFrameIndex"
               OpMemberName %type_View 87 "View_CameraCut"
               OpMemberName %type_View 88 "View_UnlitViewmodeMask"
               OpMemberName %type_View 89 "PrePadding_View_2164"
               OpMemberName %type_View 90 "PrePadding_View_2168"
               OpMemberName %type_View 91 "PrePadding_View_2172"
               OpMemberName %type_View 92 "View_DirectionalLightColor"
               OpMemberName %type_View 93 "View_DirectionalLightDirection"
               OpMemberName %type_View 94 "PrePadding_View_2204"
               OpMemberName %type_View 95 "View_TranslucencyLightingVolumeMin"
               OpMemberName %type_View 96 "View_TranslucencyLightingVolumeInvSize"
               OpMemberName %type_View 97 "View_TemporalAAParams"
               OpMemberName %type_View 98 "View_CircleDOFParams"
               OpMemberName %type_View 99 "View_DepthOfFieldSensorWidth"
               OpMemberName %type_View 100 "View_DepthOfFieldFocalDistance"
               OpMemberName %type_View 101 "View_DepthOfFieldScale"
               OpMemberName %type_View 102 "View_DepthOfFieldFocalLength"
               OpMemberName %type_View 103 "View_DepthOfFieldFocalRegion"
               OpMemberName %type_View 104 "View_DepthOfFieldNearTransitionRegion"
               OpMemberName %type_View 105 "View_DepthOfFieldFarTransitionRegion"
               OpMemberName %type_View 106 "View_MotionBlurNormalizedToPixel"
               OpMemberName %type_View 107 "View_bSubsurfacePostprocessEnabled"
               OpMemberName %type_View 108 "View_GeneralPurposeTweak"
               OpMemberName %type_View 109 "View_DemosaicVposOffset"
               OpMemberName %type_View 110 "PrePadding_View_2348"
               OpMemberName %type_View 111 "View_IndirectLightingColorScale"
               OpMemberName %type_View 112 "View_HDR32bppEncodingMode"
               OpMemberName %type_View 113 "View_AtmosphericFogSunDirection"
               OpMemberName %type_View 114 "View_AtmosphericFogSunPower"
               OpMemberName %type_View 115 "View_AtmosphericFogPower"
               OpMemberName %type_View 116 "View_AtmosphericFogDensityScale"
               OpMemberName %type_View 117 "View_AtmosphericFogDensityOffset"
               OpMemberName %type_View 118 "View_AtmosphericFogGroundOffset"
               OpMemberName %type_View 119 "View_AtmosphericFogDistanceScale"
               OpMemberName %type_View 120 "View_AtmosphericFogAltitudeScale"
               OpMemberName %type_View 121 "View_AtmosphericFogHeightScaleRayleigh"
               OpMemberName %type_View 122 "View_AtmosphericFogStartDistance"
               OpMemberName %type_View 123 "View_AtmosphericFogDistanceOffset"
               OpMemberName %type_View 124 "View_AtmosphericFogSunDiscScale"
               OpMemberName %type_View 125 "View_AtmosphericFogRenderMask"
               OpMemberName %type_View 126 "View_AtmosphericFogInscatterAltitudeSampleNum"
               OpMemberName %type_View 127 "View_AtmosphericFogSunColor"
               OpMemberName %type_View 128 "View_NormalCurvatureToRoughnessScaleBias"
               OpMemberName %type_View 129 "View_RenderingReflectionCaptureMask"
               OpMemberName %type_View 130 "View_AmbientCubemapTint"
               OpMemberName %type_View 131 "View_AmbientCubemapIntensity"
               OpMemberName %type_View 132 "View_SkyLightParameters"
               OpMemberName %type_View 133 "PrePadding_View_2488"
               OpMemberName %type_View 134 "PrePadding_View_2492"
               OpMemberName %type_View 135 "View_SkyLightColor"
               OpMemberName %type_View 136 "View_SkyIrradianceEnvironmentMap"
               OpMemberName %type_View 137 "View_MobilePreviewMode"
               OpMemberName %type_View 138 "View_HMDEyePaddingOffset"
               OpMemberName %type_View 139 "View_ReflectionCubemapMaxMip"
               OpMemberName %type_View 140 "View_ShowDecalsMask"
               OpMemberName %type_View 141 "View_DistanceFieldAOSpecularOcclusionMode"
               OpMemberName %type_View 142 "View_IndirectCapsuleSelfShadowingIntensity"
               OpMemberName %type_View 143 "PrePadding_View_2648"
               OpMemberName %type_View 144 "PrePadding_View_2652"
               OpMemberName %type_View 145 "View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight"
               OpMemberName %type_View 146 "View_StereoPassIndex"
               OpMemberName %type_View 147 "View_GlobalVolumeCenterAndExtent"
               OpMemberName %type_View 148 "View_GlobalVolumeWorldToUVAddAndMul"
               OpMemberName %type_View 149 "View_GlobalVolumeDimension"
               OpMemberName %type_View 150 "View_GlobalVolumeTexelSize"
               OpMemberName %type_View 151 "View_MaxGlobalDistance"
               OpMemberName %type_View 152 "View_bCheckerboardSubsurfaceProfileRendering"
               OpMemberName %type_View 153 "View_VolumetricFogInvGridSize"
               OpMemberName %type_View 154 "PrePadding_View_2828"
               OpMemberName %type_View 155 "View_VolumetricFogGridZParams"
               OpMemberName %type_View 156 "PrePadding_View_2844"
               OpMemberName %type_View 157 "View_VolumetricFogSVPosToVolumeUV"
               OpMemberName %type_View 158 "View_VolumetricFogMaxDistance"
               OpMemberName %type_View 159 "PrePadding_View_2860"
               OpMemberName %type_View 160 "View_VolumetricLightmapWorldToUVScale"
               OpMemberName %type_View 161 "PrePadding_View_2876"
               OpMemberName %type_View 162 "View_VolumetricLightmapWorldToUVAdd"
               OpMemberName %type_View 163 "PrePadding_View_2892"
               OpMemberName %type_View 164 "View_VolumetricLightmapIndirectionTextureSize"
               OpMemberName %type_View 165 "View_VolumetricLightmapBrickSize"
               OpMemberName %type_View 166 "View_VolumetricLightmapBrickTexelSize"
               OpMemberName %type_View 167 "View_StereoIPD"
               OpMemberName %type_View 168 "View_IndirectLightingCacheShowFlag"
               OpMemberName %type_View 169 "View_EyeToPixelSpreadAngle"
               OpName %View "View"
               OpName %type_StructuredBuffer_v4float "type.StructuredBuffer.v4float"
               OpName %View_PrimitiveSceneData "View_PrimitiveSceneData"
               OpName %in_var_TEXCOORD10_centroid "in.var.TEXCOORD10_centroid"
               OpName %in_var_TEXCOORD11_centroid "in.var.TEXCOORD11_centroid"
               OpName %in_var_COLOR0 "in.var.COLOR0"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %in_var_TEXCOORD4 "in.var.TEXCOORD4"
               OpName %in_var_PRIMITIVE_ID "in.var.PRIMITIVE_ID"
               OpName %in_var_LIGHTMAP_ID "in.var.LIGHTMAP_ID"
               OpName %in_var_VS_To_DS_Position "in.var.VS_To_DS_Position"
               OpName %out_var_TEXCOORD10_centroid "out.var.TEXCOORD10_centroid"
               OpName %out_var_TEXCOORD11_centroid "out.var.TEXCOORD11_centroid"
               OpName %out_var_COLOR0 "out.var.COLOR0"
               OpName %out_var_TEXCOORD0 "out.var.TEXCOORD0"
               OpName %out_var_TEXCOORD4 "out.var.TEXCOORD4"
               OpName %out_var_PRIMITIVE_ID "out.var.PRIMITIVE_ID"
               OpName %out_var_LIGHTMAP_ID "out.var.LIGHTMAP_ID"
               OpName %out_var_VS_To_DS_Position "out.var.VS_To_DS_Position"
               OpName %out_var_PN_POSITION "out.var.PN_POSITION"
               OpName %out_var_PN_DisplacementScales "out.var.PN_DisplacementScales"
               OpName %out_var_PN_TessellationMultiplier "out.var.PN_TessellationMultiplier"
               OpName %out_var_PN_WorldDisplacementMultiplier "out.var.PN_WorldDisplacementMultiplier"
               OpName %out_var_PN_POSITION9 "out.var.PN_POSITION9"
               OpName %MainHull "MainHull"
               OpName %param_var_I "param.var.I"
               OpName %temp_var_hullMainRetVal "temp.var.hullMainRetVal"
               OpName %if_merge "if.merge"
               OpDecorateString %in_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %in_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %in_var_COLOR0 UserSemantic "COLOR0"
               OpDecorateString %in_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %in_var_TEXCOORD4 UserSemantic "TEXCOORD4"
               OpDecorateString %in_var_PRIMITIVE_ID UserSemantic "PRIMITIVE_ID"
               OpDecorateString %in_var_LIGHTMAP_ID UserSemantic "LIGHTMAP_ID"
               OpDecorateString %in_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorateString %gl_InvocationID UserSemantic "SV_OutputControlPointID"
               OpDecorateString %out_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %out_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %out_var_COLOR0 UserSemantic "COLOR0"
               OpDecorateString %out_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %out_var_TEXCOORD4 UserSemantic "TEXCOORD4"
               OpDecorateString %out_var_PRIMITIVE_ID UserSemantic "PRIMITIVE_ID"
               OpDecorateString %out_var_LIGHTMAP_ID UserSemantic "LIGHTMAP_ID"
               OpDecorateString %out_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorateString %out_var_PN_POSITION UserSemantic "PN_POSITION"
               OpDecorateString %out_var_PN_DisplacementScales UserSemantic "PN_DisplacementScales"
               OpDecorateString %out_var_PN_TessellationMultiplier UserSemantic "PN_TessellationMultiplier"
               OpDecorateString %out_var_PN_WorldDisplacementMultiplier UserSemantic "PN_WorldDisplacementMultiplier"
               OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
               OpDecorateString %gl_TessLevelOuter UserSemantic "SV_TessFactor"
               OpDecorate %gl_TessLevelOuter Patch
               OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
               OpDecorateString %gl_TessLevelInner UserSemantic "SV_InsideTessFactor"
               OpDecorate %gl_TessLevelInner Patch
               OpDecorateString %out_var_PN_POSITION9 UserSemantic "PN_POSITION9"
               OpDecorate %out_var_PN_POSITION9 Patch
               OpDecorate %in_var_TEXCOORD10_centroid Location 0
               OpDecorate %in_var_TEXCOORD11_centroid Location 1
               OpDecorate %in_var_COLOR0 Location 2
               OpDecorate %in_var_TEXCOORD0 Location 3
               OpDecorate %in_var_TEXCOORD4 Location 4
               OpDecorate %in_var_PRIMITIVE_ID Location 5
               OpDecorate %in_var_LIGHTMAP_ID Location 6
               OpDecorate %in_var_VS_To_DS_Position Location 7
               OpDecorate %out_var_COLOR0 Location 0
               OpDecorate %out_var_LIGHTMAP_ID Location 1
               OpDecorate %out_var_PN_DisplacementScales Location 2
               OpDecorate %out_var_PN_POSITION Location 3
               OpDecorate %out_var_PN_POSITION9 Location 6
               OpDecorate %out_var_PN_TessellationMultiplier Location 7
               OpDecorate %out_var_PN_WorldDisplacementMultiplier Location 8
               OpDecorate %out_var_PRIMITIVE_ID Location 9
               OpDecorate %out_var_TEXCOORD0 Location 10
               OpDecorate %out_var_TEXCOORD10_centroid Location 11
               OpDecorate %out_var_TEXCOORD11_centroid Location 12
               OpDecorate %out_var_TEXCOORD4 Location 13
               OpDecorate %out_var_VS_To_DS_Position Location 14
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 1
               OpDecorate %View_PrimitiveSceneData DescriptorSet 0
               OpDecorate %View_PrimitiveSceneData Binding 0
               OpDecorate %_arr_v4float_uint_4 ArrayStride 16
               OpDecorate %_arr_v4float_uint_2 ArrayStride 16
               OpDecorate %_arr_v4float_uint_7 ArrayStride 16
               OpMemberDecorate %type_View 0 Offset 0
               OpMemberDecorate %type_View 0 MatrixStride 16
               OpMemberDecorate %type_View 0 ColMajor
               OpMemberDecorate %type_View 1 Offset 64
               OpMemberDecorate %type_View 1 MatrixStride 16
               OpMemberDecorate %type_View 1 ColMajor
               OpMemberDecorate %type_View 2 Offset 128
               OpMemberDecorate %type_View 2 MatrixStride 16
               OpMemberDecorate %type_View 2 ColMajor
               OpMemberDecorate %type_View 3 Offset 192
               OpMemberDecorate %type_View 3 MatrixStride 16
               OpMemberDecorate %type_View 3 ColMajor
               OpMemberDecorate %type_View 4 Offset 256
               OpMemberDecorate %type_View 4 MatrixStride 16
               OpMemberDecorate %type_View 4 ColMajor
               OpMemberDecorate %type_View 5 Offset 320
               OpMemberDecorate %type_View 5 MatrixStride 16
               OpMemberDecorate %type_View 5 ColMajor
               OpMemberDecorate %type_View 6 Offset 384
               OpMemberDecorate %type_View 6 MatrixStride 16
               OpMemberDecorate %type_View 6 ColMajor
               OpMemberDecorate %type_View 7 Offset 448
               OpMemberDecorate %type_View 7 MatrixStride 16
               OpMemberDecorate %type_View 7 ColMajor
               OpMemberDecorate %type_View 8 Offset 512
               OpMemberDecorate %type_View 8 MatrixStride 16
               OpMemberDecorate %type_View 8 ColMajor
               OpMemberDecorate %type_View 9 Offset 576
               OpMemberDecorate %type_View 9 MatrixStride 16
               OpMemberDecorate %type_View 9 ColMajor
               OpMemberDecorate %type_View 10 Offset 640
               OpMemberDecorate %type_View 10 MatrixStride 16
               OpMemberDecorate %type_View 10 ColMajor
               OpMemberDecorate %type_View 11 Offset 704
               OpMemberDecorate %type_View 11 MatrixStride 16
               OpMemberDecorate %type_View 11 ColMajor
               OpMemberDecorate %type_View 12 Offset 768
               OpMemberDecorate %type_View 12 MatrixStride 16
               OpMemberDecorate %type_View 12 ColMajor
               OpMemberDecorate %type_View 13 Offset 832
               OpMemberDecorate %type_View 14 Offset 844
               OpMemberDecorate %type_View 15 Offset 848
               OpMemberDecorate %type_View 16 Offset 860
               OpMemberDecorate %type_View 17 Offset 864
               OpMemberDecorate %type_View 18 Offset 876
               OpMemberDecorate %type_View 19 Offset 880
               OpMemberDecorate %type_View 20 Offset 892
               OpMemberDecorate %type_View 21 Offset 896
               OpMemberDecorate %type_View 22 Offset 908
               OpMemberDecorate %type_View 23 Offset 912
               OpMemberDecorate %type_View 24 Offset 928
               OpMemberDecorate %type_View 25 Offset 944
               OpMemberDecorate %type_View 26 Offset 956
               OpMemberDecorate %type_View 27 Offset 960
               OpMemberDecorate %type_View 28 Offset 972
               OpMemberDecorate %type_View 29 Offset 976
               OpMemberDecorate %type_View 30 Offset 988
               OpMemberDecorate %type_View 31 Offset 992
               OpMemberDecorate %type_View 32 Offset 1004
               OpMemberDecorate %type_View 33 Offset 1008
               OpMemberDecorate %type_View 33 MatrixStride 16
               OpMemberDecorate %type_View 33 ColMajor
               OpMemberDecorate %type_View 34 Offset 1072
               OpMemberDecorate %type_View 34 MatrixStride 16
               OpMemberDecorate %type_View 34 ColMajor
               OpMemberDecorate %type_View 35 Offset 1136
               OpMemberDecorate %type_View 35 MatrixStride 16
               OpMemberDecorate %type_View 35 ColMajor
               OpMemberDecorate %type_View 36 Offset 1200
               OpMemberDecorate %type_View 36 MatrixStride 16
               OpMemberDecorate %type_View 36 ColMajor
               OpMemberDecorate %type_View 37 Offset 1264
               OpMemberDecorate %type_View 37 MatrixStride 16
               OpMemberDecorate %type_View 37 ColMajor
               OpMemberDecorate %type_View 38 Offset 1328
               OpMemberDecorate %type_View 38 MatrixStride 16
               OpMemberDecorate %type_View 38 ColMajor
               OpMemberDecorate %type_View 39 Offset 1392
               OpMemberDecorate %type_View 39 MatrixStride 16
               OpMemberDecorate %type_View 39 ColMajor
               OpMemberDecorate %type_View 40 Offset 1456
               OpMemberDecorate %type_View 40 MatrixStride 16
               OpMemberDecorate %type_View 40 ColMajor
               OpMemberDecorate %type_View 41 Offset 1520
               OpMemberDecorate %type_View 41 MatrixStride 16
               OpMemberDecorate %type_View 41 ColMajor
               OpMemberDecorate %type_View 42 Offset 1584
               OpMemberDecorate %type_View 42 MatrixStride 16
               OpMemberDecorate %type_View 42 ColMajor
               OpMemberDecorate %type_View 43 Offset 1648
               OpMemberDecorate %type_View 44 Offset 1660
               OpMemberDecorate %type_View 45 Offset 1664
               OpMemberDecorate %type_View 46 Offset 1676
               OpMemberDecorate %type_View 47 Offset 1680
               OpMemberDecorate %type_View 48 Offset 1692
               OpMemberDecorate %type_View 49 Offset 1696
               OpMemberDecorate %type_View 49 MatrixStride 16
               OpMemberDecorate %type_View 49 ColMajor
               OpMemberDecorate %type_View 50 Offset 1760
               OpMemberDecorate %type_View 50 MatrixStride 16
               OpMemberDecorate %type_View 50 ColMajor
               OpMemberDecorate %type_View 51 Offset 1824
               OpMemberDecorate %type_View 51 MatrixStride 16
               OpMemberDecorate %type_View 51 ColMajor
               OpMemberDecorate %type_View 52 Offset 1888
               OpMemberDecorate %type_View 53 Offset 1904
               OpMemberDecorate %type_View 54 Offset 1920
               OpMemberDecorate %type_View 55 Offset 1928
               OpMemberDecorate %type_View 56 Offset 1936
               OpMemberDecorate %type_View 57 Offset 1952
               OpMemberDecorate %type_View 58 Offset 1968
               OpMemberDecorate %type_View 59 Offset 1984
               OpMemberDecorate %type_View 60 Offset 2000
               OpMemberDecorate %type_View 61 Offset 2004
               OpMemberDecorate %type_View 62 Offset 2008
               OpMemberDecorate %type_View 63 Offset 2012
               OpMemberDecorate %type_View 64 Offset 2016
               OpMemberDecorate %type_View 65 Offset 2032
               OpMemberDecorate %type_View 66 Offset 2048
               OpMemberDecorate %type_View 67 Offset 2064
               OpMemberDecorate %type_View 68 Offset 2072
               OpMemberDecorate %type_View 69 Offset 2076
               OpMemberDecorate %type_View 70 Offset 2080
               OpMemberDecorate %type_View 71 Offset 2084
               OpMemberDecorate %type_View 72 Offset 2088
               OpMemberDecorate %type_View 73 Offset 2092
               OpMemberDecorate %type_View 74 Offset 2096
               OpMemberDecorate %type_View 75 Offset 2108
               OpMemberDecorate %type_View 76 Offset 2112
               OpMemberDecorate %type_View 77 Offset 2116
               OpMemberDecorate %type_View 78 Offset 2120
               OpMemberDecorate %type_View 79 Offset 2124
               OpMemberDecorate %type_View 80 Offset 2128
               OpMemberDecorate %type_View 81 Offset 2132
               OpMemberDecorate %type_View 82 Offset 2136
               OpMemberDecorate %type_View 83 Offset 2140
               OpMemberDecorate %type_View 84 Offset 2144
               OpMemberDecorate %type_View 85 Offset 2148
               OpMemberDecorate %type_View 86 Offset 2152
               OpMemberDecorate %type_View 87 Offset 2156
               OpMemberDecorate %type_View 88 Offset 2160
               OpMemberDecorate %type_View 89 Offset 2164
               OpMemberDecorate %type_View 90 Offset 2168
               OpMemberDecorate %type_View 91 Offset 2172
               OpMemberDecorate %type_View 92 Offset 2176
               OpMemberDecorate %type_View 93 Offset 2192
               OpMemberDecorate %type_View 94 Offset 2204
               OpMemberDecorate %type_View 95 Offset 2208
               OpMemberDecorate %type_View 96 Offset 2240
               OpMemberDecorate %type_View 97 Offset 2272
               OpMemberDecorate %type_View 98 Offset 2288
               OpMemberDecorate %type_View 99 Offset 2304
               OpMemberDecorate %type_View 100 Offset 2308
               OpMemberDecorate %type_View 101 Offset 2312
               OpMemberDecorate %type_View 102 Offset 2316
               OpMemberDecorate %type_View 103 Offset 2320
               OpMemberDecorate %type_View 104 Offset 2324
               OpMemberDecorate %type_View 105 Offset 2328
               OpMemberDecorate %type_View 106 Offset 2332
               OpMemberDecorate %type_View 107 Offset 2336
               OpMemberDecorate %type_View 108 Offset 2340
               OpMemberDecorate %type_View 109 Offset 2344
               OpMemberDecorate %type_View 110 Offset 2348
               OpMemberDecorate %type_View 111 Offset 2352
               OpMemberDecorate %type_View 112 Offset 2364
               OpMemberDecorate %type_View 113 Offset 2368
               OpMemberDecorate %type_View 114 Offset 2380
               OpMemberDecorate %type_View 115 Offset 2384
               OpMemberDecorate %type_View 116 Offset 2388
               OpMemberDecorate %type_View 117 Offset 2392
               OpMemberDecorate %type_View 118 Offset 2396
               OpMemberDecorate %type_View 119 Offset 2400
               OpMemberDecorate %type_View 120 Offset 2404
               OpMemberDecorate %type_View 121 Offset 2408
               OpMemberDecorate %type_View 122 Offset 2412
               OpMemberDecorate %type_View 123 Offset 2416
               OpMemberDecorate %type_View 124 Offset 2420
               OpMemberDecorate %type_View 125 Offset 2424
               OpMemberDecorate %type_View 126 Offset 2428
               OpMemberDecorate %type_View 127 Offset 2432
               OpMemberDecorate %type_View 128 Offset 2448
               OpMemberDecorate %type_View 129 Offset 2460
               OpMemberDecorate %type_View 130 Offset 2464
               OpMemberDecorate %type_View 131 Offset 2480
               OpMemberDecorate %type_View 132 Offset 2484
               OpMemberDecorate %type_View 133 Offset 2488
               OpMemberDecorate %type_View 134 Offset 2492
               OpMemberDecorate %type_View 135 Offset 2496
               OpMemberDecorate %type_View 136 Offset 2512
               OpMemberDecorate %type_View 137 Offset 2624
               OpMemberDecorate %type_View 138 Offset 2628
               OpMemberDecorate %type_View 139 Offset 2632
               OpMemberDecorate %type_View 140 Offset 2636
               OpMemberDecorate %type_View 141 Offset 2640
               OpMemberDecorate %type_View 142 Offset 2644
               OpMemberDecorate %type_View 143 Offset 2648
               OpMemberDecorate %type_View 144 Offset 2652
               OpMemberDecorate %type_View 145 Offset 2656
               OpMemberDecorate %type_View 146 Offset 2668
               OpMemberDecorate %type_View 147 Offset 2672
               OpMemberDecorate %type_View 148 Offset 2736
               OpMemberDecorate %type_View 149 Offset 2800
               OpMemberDecorate %type_View 150 Offset 2804
               OpMemberDecorate %type_View 151 Offset 2808
               OpMemberDecorate %type_View 152 Offset 2812
               OpMemberDecorate %type_View 153 Offset 2816
               OpMemberDecorate %type_View 154 Offset 2828
               OpMemberDecorate %type_View 155 Offset 2832
               OpMemberDecorate %type_View 156 Offset 2844
               OpMemberDecorate %type_View 157 Offset 2848
               OpMemberDecorate %type_View 158 Offset 2856
               OpMemberDecorate %type_View 159 Offset 2860
               OpMemberDecorate %type_View 160 Offset 2864
               OpMemberDecorate %type_View 161 Offset 2876
               OpMemberDecorate %type_View 162 Offset 2880
               OpMemberDecorate %type_View 163 Offset 2892
               OpMemberDecorate %type_View 164 Offset 2896
               OpMemberDecorate %type_View 165 Offset 2908
               OpMemberDecorate %type_View 166 Offset 2912
               OpMemberDecorate %type_View 167 Offset 2924
               OpMemberDecorate %type_View 168 Offset 2928
               OpMemberDecorate %type_View 169 Offset 2932
               OpDecorate %type_View Block
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_v4float 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_v4float 0 NonWritable
               OpDecorate %type_StructuredBuffer_v4float BufferBlock
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
     %uint_7 = OpConstant %uint 7
     %uint_4 = OpConstant %uint 4
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
    %float_2 = OpConstant %float 2
         %62 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
  %float_0_5 = OpConstant %float 0.5
      %int_3 = OpConstant %int 3
%float_0_333000004 = OpConstant %float 0.333000004
    %float_1 = OpConstant %float 1
         %67 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
   %float_15 = OpConstant %float 15
         %69 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%FVertexFactoryInterpolantsVSToPS = OpTypeStruct %v4float %v4float %v4float %_arr_v4float_uint_1 %v4float %uint %uint
%FVertexFactoryInterpolantsVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToPS
%FSharedBasePassInterpolants = OpTypeStruct
%FBasePassInterpolantsVSToDS = OpTypeStruct %FSharedBasePassInterpolants
%FBasePassVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToDS %FBasePassInterpolantsVSToDS %v4float
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%FPNTessellationHSToDS = OpTypeStruct %FBasePassVSToDS %_arr_v4float_uint_3 %v3float %float %float
      %v3int = OpTypeVector %int 3
         %73 = OpConstantComposite %v3int %int_0 %int_0 %int_0
         %74 = OpConstantComposite %v3int %int_3 %int_3 %int_3
    %float_0 = OpConstant %float 0
         %76 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %77 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %int_77 = OpConstant %int 77
      %int_6 = OpConstant %int 6
     %int_27 = OpConstant %int 27
         %81 = OpConstantComposite %v3int %int_1 %int_1 %int_1
         %82 = OpConstantComposite %v3int %int_2 %int_2 %int_2
    %uint_26 = OpConstant %uint 26
    %uint_12 = OpConstant %uint 12
    %uint_22 = OpConstant %uint 22
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
%type_StructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
%_ptr_Uniform_type_StructuredBuffer_v4float = OpTypePointer Uniform %type_StructuredBuffer_v4float
%_arr_v4float_uint_12 = OpTypeArray %v4float %uint_12
%_ptr_Input__arr_v4float_uint_12 = OpTypePointer Input %_arr_v4float_uint_12
%_arr__arr_v4float_uint_1_uint_12 = OpTypeArray %_arr_v4float_uint_1 %uint_12
%_ptr_Input__arr__arr_v4float_uint_1_uint_12 = OpTypePointer Input %_arr__arr_v4float_uint_1_uint_12
%_arr_uint_uint_12 = OpTypeArray %uint %uint_12
%_ptr_Input__arr_uint_uint_12 = OpTypePointer Input %_arr_uint_uint_12
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
%_arr__arr_v4float_uint_1_uint_3 = OpTypeArray %_arr_v4float_uint_1 %uint_3
%_ptr_Output__arr__arr_v4float_uint_1_uint_3 = OpTypePointer Output %_arr__arr_v4float_uint_1_uint_3
%_arr_uint_uint_3 = OpTypeArray %uint %uint_3
%_ptr_Output__arr_uint_uint_3 = OpTypePointer Output %_arr_uint_uint_3
%_arr__arr_v4float_uint_3_uint_3 = OpTypeArray %_arr_v4float_uint_3 %uint_3
%_ptr_Output__arr__arr_v4float_uint_3_uint_3 = OpTypePointer Output %_arr__arr_v4float_uint_3_uint_3
%_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
%_ptr_Output__arr_v3float_uint_3 = OpTypePointer Output %_arr_v3float_uint_3
%_ptr_Output__arr_float_uint_3 = OpTypePointer Output %_arr_float_uint_3
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
        %111 = OpTypeFunction %void
%_arr_FBasePassVSToDS_uint_12 = OpTypeArray %FBasePassVSToDS %uint_12
%_ptr_Function__arr_FBasePassVSToDS_uint_12 = OpTypePointer Function %_arr_FBasePassVSToDS_uint_12
%_arr_FPNTessellationHSToDS_uint_3 = OpTypeArray %FPNTessellationHSToDS %uint_3
%_ptr_Workgroup__arr_FPNTessellationHSToDS_uint_3 = OpTypePointer Workgroup %_arr_FPNTessellationHSToDS_uint_3
%_ptr_Output__arr_v4float_uint_1 = OpTypePointer Output %_arr_v4float_uint_1
%_ptr_Output_uint = OpTypePointer Output %uint
%_ptr_Output_v3float = OpTypePointer Output %v3float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Workgroup_FPNTessellationHSToDS = OpTypePointer Workgroup %FPNTessellationHSToDS
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Workgroup_v4float = OpTypePointer Workgroup %v4float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Workgroup_float = OpTypePointer Workgroup %float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_FVertexFactoryInterpolantsVSToDS = OpTypePointer Function %FVertexFactoryInterpolantsVSToDS
%_ptr_Function_FVertexFactoryInterpolantsVSToPS = OpTypePointer Function %FVertexFactoryInterpolantsVSToPS
%_ptr_Function_FBasePassVSToDS = OpTypePointer Function %FBasePassVSToDS
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
       %View = OpVariable %_ptr_Uniform_type_View Uniform
%View_PrimitiveSceneData = OpVariable %_ptr_Uniform_type_StructuredBuffer_v4float Uniform
%in_var_TEXCOORD10_centroid = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_TEXCOORD11_centroid = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_COLOR0 = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_TEXCOORD0 = OpVariable %_ptr_Input__arr__arr_v4float_uint_1_uint_12 Input
%in_var_TEXCOORD4 = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_PRIMITIVE_ID = OpVariable %_ptr_Input__arr_uint_uint_12 Input
%in_var_LIGHTMAP_ID = OpVariable %_ptr_Input__arr_uint_uint_12 Input
%in_var_VS_To_DS_Position = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%gl_InvocationID = OpVariable %_ptr_Input_uint Input
%out_var_TEXCOORD10_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_TEXCOORD11_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_COLOR0 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_TEXCOORD0 = OpVariable %_ptr_Output__arr__arr_v4float_uint_1_uint_3 Output
%out_var_TEXCOORD4 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_PRIMITIVE_ID = OpVariable %_ptr_Output__arr_uint_uint_3 Output
%out_var_LIGHTMAP_ID = OpVariable %_ptr_Output__arr_uint_uint_3 Output
%out_var_VS_To_DS_Position = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_PN_POSITION = OpVariable %_ptr_Output__arr__arr_v4float_uint_3_uint_3 Output
%out_var_PN_DisplacementScales = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%out_var_PN_TessellationMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%out_var_PN_WorldDisplacementMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
%out_var_PN_POSITION9 = OpVariable %_ptr_Output_v4float Output
        %133 = OpConstantNull %FSharedBasePassInterpolants
        %134 = OpConstantComposite %FBasePassInterpolantsVSToDS %133
%float_0_333333343 = OpConstant %float 0.333333343
        %136 = OpConstantComposite %v4float %float_0_333333343 %float_0_333333343 %float_0_333333343 %float_0_333333343
        %137 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%float_0_166666672 = OpConstant %float 0.166666672
        %139 = OpConstantComposite %v4float %float_0_166666672 %float_0_166666672 %float_0_166666672 %float_0_166666672
        %140 = OpUndef %v4float

; XXX: Original asm used Function here, which is wrong.
; This patches the SPIR-V to be correct.
%temp_var_hullMainRetVal = OpVariable %_ptr_Workgroup__arr_FPNTessellationHSToDS_uint_3 Workgroup

   %MainHull = OpFunction %void None %111
        %141 = OpLabel
%param_var_I = OpVariable %_ptr_Function__arr_FBasePassVSToDS_uint_12 Function
        %142 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD10_centroid
        %143 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD11_centroid
        %144 = OpLoad %_arr_v4float_uint_12 %in_var_COLOR0
        %145 = OpLoad %_arr__arr_v4float_uint_1_uint_12 %in_var_TEXCOORD0
        %146 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD4
        %147 = OpLoad %_arr_uint_uint_12 %in_var_PRIMITIVE_ID
        %148 = OpLoad %_arr_uint_uint_12 %in_var_LIGHTMAP_ID
        %149 = OpCompositeExtract %v4float %142 0
        %150 = OpCompositeExtract %v4float %143 0
        %151 = OpCompositeExtract %v4float %144 0
        %152 = OpCompositeExtract %_arr_v4float_uint_1 %145 0
        %153 = OpCompositeExtract %v4float %146 0
        %154 = OpCompositeExtract %uint %147 0
        %155 = OpCompositeExtract %uint %148 0
        %156 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %149 %150 %151 %152 %153 %154 %155
        %157 = OpCompositeExtract %v4float %142 1
        %158 = OpCompositeExtract %v4float %143 1
        %159 = OpCompositeExtract %v4float %144 1
        %160 = OpCompositeExtract %_arr_v4float_uint_1 %145 1
        %161 = OpCompositeExtract %v4float %146 1
        %162 = OpCompositeExtract %uint %147 1
        %163 = OpCompositeExtract %uint %148 1
        %164 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %157 %158 %159 %160 %161 %162 %163
        %165 = OpCompositeExtract %v4float %142 2
        %166 = OpCompositeExtract %v4float %143 2
        %167 = OpCompositeExtract %v4float %144 2
        %168 = OpCompositeExtract %_arr_v4float_uint_1 %145 2
        %169 = OpCompositeExtract %v4float %146 2
        %170 = OpCompositeExtract %uint %147 2
        %171 = OpCompositeExtract %uint %148 2
        %172 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %165 %166 %167 %168 %169 %170 %171
        %173 = OpCompositeExtract %v4float %142 3
        %174 = OpCompositeExtract %v4float %143 3
        %175 = OpCompositeExtract %v4float %144 3
        %176 = OpCompositeExtract %_arr_v4float_uint_1 %145 3
        %177 = OpCompositeExtract %v4float %146 3
        %178 = OpCompositeExtract %uint %147 3
        %179 = OpCompositeExtract %uint %148 3
        %180 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %173 %174 %175 %176 %177 %178 %179
        %181 = OpCompositeExtract %v4float %142 4
        %182 = OpCompositeExtract %v4float %143 4
        %183 = OpCompositeExtract %v4float %144 4
        %184 = OpCompositeExtract %_arr_v4float_uint_1 %145 4
        %185 = OpCompositeExtract %v4float %146 4
        %186 = OpCompositeExtract %uint %147 4
        %187 = OpCompositeExtract %uint %148 4
        %188 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %181 %182 %183 %184 %185 %186 %187
        %189 = OpCompositeExtract %v4float %142 5
        %190 = OpCompositeExtract %v4float %143 5
        %191 = OpCompositeExtract %v4float %144 5
        %192 = OpCompositeExtract %_arr_v4float_uint_1 %145 5
        %193 = OpCompositeExtract %v4float %146 5
        %194 = OpCompositeExtract %uint %147 5
        %195 = OpCompositeExtract %uint %148 5
        %196 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %189 %190 %191 %192 %193 %194 %195
        %197 = OpCompositeExtract %v4float %142 6
        %198 = OpCompositeExtract %v4float %143 6
        %199 = OpCompositeExtract %v4float %144 6
        %200 = OpCompositeExtract %_arr_v4float_uint_1 %145 6
        %201 = OpCompositeExtract %v4float %146 6
        %202 = OpCompositeExtract %uint %147 6
        %203 = OpCompositeExtract %uint %148 6
        %204 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %197 %198 %199 %200 %201 %202 %203
        %205 = OpCompositeExtract %v4float %142 7
        %206 = OpCompositeExtract %v4float %143 7
        %207 = OpCompositeExtract %v4float %144 7
        %208 = OpCompositeExtract %_arr_v4float_uint_1 %145 7
        %209 = OpCompositeExtract %v4float %146 7
        %210 = OpCompositeExtract %uint %147 7
        %211 = OpCompositeExtract %uint %148 7
        %212 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %205 %206 %207 %208 %209 %210 %211
        %213 = OpCompositeExtract %v4float %142 8
        %214 = OpCompositeExtract %v4float %143 8
        %215 = OpCompositeExtract %v4float %144 8
        %216 = OpCompositeExtract %_arr_v4float_uint_1 %145 8
        %217 = OpCompositeExtract %v4float %146 8
        %218 = OpCompositeExtract %uint %147 8
        %219 = OpCompositeExtract %uint %148 8
        %220 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %213 %214 %215 %216 %217 %218 %219
        %221 = OpCompositeExtract %v4float %142 9
        %222 = OpCompositeExtract %v4float %143 9
        %223 = OpCompositeExtract %v4float %144 9
        %224 = OpCompositeExtract %_arr_v4float_uint_1 %145 9
        %225 = OpCompositeExtract %v4float %146 9
        %226 = OpCompositeExtract %uint %147 9
        %227 = OpCompositeExtract %uint %148 9
        %228 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %221 %222 %223 %224 %225 %226 %227
        %229 = OpCompositeExtract %v4float %142 10
        %230 = OpCompositeExtract %v4float %143 10
        %231 = OpCompositeExtract %v4float %144 10
        %232 = OpCompositeExtract %_arr_v4float_uint_1 %145 10
        %233 = OpCompositeExtract %v4float %146 10
        %234 = OpCompositeExtract %uint %147 10
        %235 = OpCompositeExtract %uint %148 10
        %236 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %229 %230 %231 %232 %233 %234 %235
        %237 = OpCompositeExtract %v4float %142 11
        %238 = OpCompositeExtract %v4float %143 11
        %239 = OpCompositeExtract %v4float %144 11
        %240 = OpCompositeExtract %_arr_v4float_uint_1 %145 11
        %241 = OpCompositeExtract %v4float %146 11
        %242 = OpCompositeExtract %uint %147 11
        %243 = OpCompositeExtract %uint %148 11
        %244 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %237 %238 %239 %240 %241 %242 %243
        %245 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %156
        %246 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %164
        %247 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %172
        %248 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %180
        %249 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %188
        %250 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %196
        %251 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %204
        %252 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %212
        %253 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %220
        %254 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %228
        %255 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %236
        %256 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %244
        %257 = OpLoad %_arr_v4float_uint_12 %in_var_VS_To_DS_Position
        %258 = OpCompositeExtract %v4float %257 0
        %259 = OpCompositeConstruct %FBasePassVSToDS %245 %134 %258
        %260 = OpCompositeExtract %v4float %257 1
        %261 = OpCompositeConstruct %FBasePassVSToDS %246 %134 %260
        %262 = OpCompositeExtract %v4float %257 2
        %263 = OpCompositeConstruct %FBasePassVSToDS %247 %134 %262
        %264 = OpCompositeExtract %v4float %257 3
        %265 = OpCompositeConstruct %FBasePassVSToDS %248 %134 %264
        %266 = OpCompositeExtract %v4float %257 4
        %267 = OpCompositeConstruct %FBasePassVSToDS %249 %134 %266
        %268 = OpCompositeExtract %v4float %257 5
        %269 = OpCompositeConstruct %FBasePassVSToDS %250 %134 %268
        %270 = OpCompositeExtract %v4float %257 6
        %271 = OpCompositeConstruct %FBasePassVSToDS %251 %134 %270
        %272 = OpCompositeExtract %v4float %257 7
        %273 = OpCompositeConstruct %FBasePassVSToDS %252 %134 %272
        %274 = OpCompositeExtract %v4float %257 8
        %275 = OpCompositeConstruct %FBasePassVSToDS %253 %134 %274
        %276 = OpCompositeExtract %v4float %257 9
        %277 = OpCompositeConstruct %FBasePassVSToDS %254 %134 %276
        %278 = OpCompositeExtract %v4float %257 10
        %279 = OpCompositeConstruct %FBasePassVSToDS %255 %134 %278
        %280 = OpCompositeExtract %v4float %257 11
        %281 = OpCompositeConstruct %FBasePassVSToDS %256 %134 %280
        %282 = OpCompositeConstruct %_arr_FBasePassVSToDS_uint_12 %259 %261 %263 %265 %267 %269 %271 %273 %275 %277 %279 %281
               OpStore %param_var_I %282
        %283 = OpLoad %uint %gl_InvocationID
        %284 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %283 %int_0
        %285 = OpLoad %FVertexFactoryInterpolantsVSToDS %284
        %286 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %285 0
        %287 = OpCompositeExtract %v4float %286 0
        %288 = OpCompositeExtract %v4float %286 1
        %289 = OpVectorShuffle %v3float %287 %287 0 1 2
        %290 = OpVectorShuffle %v3float %288 %288 0 1 2
        %291 = OpExtInst %v3float %1 Cross %290 %289
        %292 = OpCompositeExtract %float %288 3
        %293 = OpCompositeConstruct %v3float %292 %292 %292
        %294 = OpFMul %v3float %291 %293
        %295 = OpCompositeConstruct %mat3v3float %289 %294 %290
        %296 = OpCompositeExtract %float %288 0
        %297 = OpCompositeExtract %float %288 1
        %298 = OpCompositeExtract %float %288 2
        %299 = OpCompositeConstruct %v4float %296 %297 %298 %float_0
        %300 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToPS %param_var_I %283 %int_0 %int_0
        %301 = OpLoad %FVertexFactoryInterpolantsVSToPS %300
        %302 = OpCompositeExtract %uint %301 5
        %303 = OpIMul %uint %302 %uint_26
        %304 = OpIAdd %uint %303 %uint_22
        %305 = OpAccessChain %_ptr_Uniform_v4float %View_PrimitiveSceneData %int_0 %304
        %306 = OpLoad %v4float %305
        %307 = OpVectorShuffle %v3float %306 %306 0 1 2
        %308 = OpVectorTimesMatrix %v3float %307 %295
        %309 = OpULessThan %bool %283 %uint_2
        %310 = OpIAdd %uint %283 %uint_1
        %311 = OpSelect %uint %309 %310 %uint_0
        %312 = OpIMul %uint %uint_2 %283
        %313 = OpIAdd %uint %uint_3 %312
        %314 = OpIAdd %uint %312 %uint_4
        %315 = OpAccessChain %_ptr_Function_FBasePassVSToDS %param_var_I %283
        %316 = OpLoad %FBasePassVSToDS %315
        %317 = OpAccessChain %_ptr_Function_v4float %param_var_I %283 %int_2
        %318 = OpLoad %v4float %317
        %319 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %311 %int_0
        %320 = OpLoad %FVertexFactoryInterpolantsVSToDS %319
        %321 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %320 0
        %322 = OpCompositeExtract %v4float %321 1
        %323 = OpCompositeExtract %float %322 0
        %324 = OpCompositeExtract %float %322 1
        %325 = OpCompositeExtract %float %322 2
        %326 = OpCompositeConstruct %v4float %323 %324 %325 %float_0
        %327 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %313 %int_0
        %328 = OpLoad %FVertexFactoryInterpolantsVSToDS %327
        %329 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %328 0
        %330 = OpCompositeExtract %v4float %329 1
        %331 = OpCompositeExtract %float %330 0
        %332 = OpCompositeExtract %float %330 1
        %333 = OpCompositeExtract %float %330 2
        %334 = OpCompositeConstruct %v4float %331 %332 %333 %float_0
        %335 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %314 %int_0
        %336 = OpLoad %FVertexFactoryInterpolantsVSToDS %335
        %337 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %336 0
        %338 = OpCompositeExtract %v4float %337 1
        %339 = OpCompositeExtract %float %338 0
        %340 = OpCompositeExtract %float %338 1
        %341 = OpCompositeExtract %float %338 2
        %342 = OpCompositeConstruct %v4float %339 %340 %341 %float_0
        %343 = OpLoad %v4float %317
        %344 = OpAccessChain %_ptr_Function_v4float %param_var_I %311 %int_2
        %345 = OpLoad %v4float %344
        %346 = OpFMul %v4float %62 %343
        %347 = OpFAdd %v4float %346 %345
        %348 = OpFSub %v4float %345 %343
        %349 = OpDot %float %348 %299
        %350 = OpCompositeConstruct %v4float %349 %349 %349 %349
        %351 = OpFMul %v4float %350 %299
        %352 = OpFSub %v4float %347 %351
        %353 = OpFMul %v4float %352 %136
        %354 = OpAccessChain %_ptr_Function_v4float %param_var_I %313 %int_2
        %355 = OpLoad %v4float %354
        %356 = OpAccessChain %_ptr_Function_v4float %param_var_I %314 %int_2
        %357 = OpLoad %v4float %356
        %358 = OpFMul %v4float %62 %355
        %359 = OpFAdd %v4float %358 %357
        %360 = OpFSub %v4float %357 %355
        %361 = OpDot %float %360 %334
        %362 = OpCompositeConstruct %v4float %361 %361 %361 %361
        %363 = OpFMul %v4float %362 %334
        %364 = OpFSub %v4float %359 %363
        %365 = OpFMul %v4float %364 %136
        %366 = OpFAdd %v4float %353 %365
        %367 = OpFMul %v4float %366 %137
        %368 = OpLoad %v4float %344
        %369 = OpLoad %v4float %317
        %370 = OpFMul %v4float %62 %368
        %371 = OpFAdd %v4float %370 %369
        %372 = OpFSub %v4float %369 %368
        %373 = OpDot %float %372 %326
        %374 = OpCompositeConstruct %v4float %373 %373 %373 %373
        %375 = OpFMul %v4float %374 %326
        %376 = OpFSub %v4float %371 %375
        %377 = OpFMul %v4float %376 %136
        %378 = OpLoad %v4float %356
        %379 = OpLoad %v4float %354
        %380 = OpFMul %v4float %62 %378
        %381 = OpFAdd %v4float %380 %379
        %382 = OpFSub %v4float %379 %378
        %383 = OpDot %float %382 %342
        %384 = OpCompositeConstruct %v4float %383 %383 %383 %383
        %385 = OpFMul %v4float %384 %342
        %386 = OpFSub %v4float %381 %385
        %387 = OpFMul %v4float %386 %136
        %388 = OpFAdd %v4float %377 %387
        %389 = OpFMul %v4float %388 %137
        %390 = OpCompositeConstruct %_arr_v4float_uint_3 %318 %367 %389
        %391 = OpCompositeConstruct %FPNTessellationHSToDS %316 %390 %308 %float_1 %float_1
        %392 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %316 0
        %393 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %392 0
        %394 = OpCompositeExtract %v4float %393 0
        %395 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD10_centroid %283
               OpStore %395 %394
        %396 = OpCompositeExtract %v4float %393 1
        %397 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD11_centroid %283
               OpStore %397 %396
        %398 = OpCompositeExtract %v4float %393 2
        %399 = OpAccessChain %_ptr_Output_v4float %out_var_COLOR0 %283
               OpStore %399 %398
        %400 = OpCompositeExtract %_arr_v4float_uint_1 %393 3
        %401 = OpAccessChain %_ptr_Output__arr_v4float_uint_1 %out_var_TEXCOORD0 %283
               OpStore %401 %400
        %402 = OpCompositeExtract %v4float %393 4
        %403 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD4 %283
               OpStore %403 %402
        %404 = OpCompositeExtract %uint %393 5
        %405 = OpAccessChain %_ptr_Output_uint %out_var_PRIMITIVE_ID %283
               OpStore %405 %404
        %406 = OpCompositeExtract %uint %393 6
        %407 = OpAccessChain %_ptr_Output_uint %out_var_LIGHTMAP_ID %283
               OpStore %407 %406
        %408 = OpCompositeExtract %v4float %316 2
        %409 = OpAccessChain %_ptr_Output_v4float %out_var_VS_To_DS_Position %283
               OpStore %409 %408
        %410 = OpAccessChain %_ptr_Output__arr_v4float_uint_3 %out_var_PN_POSITION %283
               OpStore %410 %390
        %411 = OpAccessChain %_ptr_Output_v3float %out_var_PN_DisplacementScales %283
               OpStore %411 %308
        %412 = OpAccessChain %_ptr_Output_float %out_var_PN_TessellationMultiplier %283
               OpStore %412 %float_1
        %413 = OpAccessChain %_ptr_Output_float %out_var_PN_WorldDisplacementMultiplier %283
               OpStore %413 %float_1
        %414 = OpAccessChain %_ptr_Workgroup_FPNTessellationHSToDS %temp_var_hullMainRetVal %283
               OpStore %414 %391
               OpControlBarrier %uint_2 %uint_4 %uint_0
        %415 = OpIEqual %bool %283 %uint_0
               OpSelectionMerge %if_merge None
               OpBranchConditional %415 %416 %if_merge
        %416 = OpLabel
        %417 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_0
        %418 = OpLoad %mat4v4float %417
        %419 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_6
        %420 = OpLoad %mat4v4float %419
        %421 = OpAccessChain %_ptr_Uniform_v3float %View %int_27
        %422 = OpLoad %v3float %421
        %423 = OpAccessChain %_ptr_Uniform_float %View %int_77
        %424 = OpLoad %float %423
        %425 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_0
        %426 = OpLoad %v4float %425
        %427 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_1
        %428 = OpLoad %v4float %427
        %429 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_2
        %430 = OpLoad %v4float %429
        %431 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_0
        %432 = OpLoad %v4float %431
        %433 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_1
        %434 = OpLoad %v4float %433
        %435 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_2
        %436 = OpLoad %v4float %435
        %437 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_0
        %438 = OpLoad %v4float %437
        %439 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_1
        %440 = OpLoad %v4float %439
        %441 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_2
        %442 = OpLoad %v4float %441
        %443 = OpFAdd %v4float %428 %430
        %444 = OpFAdd %v4float %443 %434
        %445 = OpFAdd %v4float %444 %436
        %446 = OpFAdd %v4float %445 %440
        %447 = OpFAdd %v4float %446 %442
        %448 = OpFMul %v4float %447 %139
        %449 = OpFAdd %v4float %438 %432
        %450 = OpFAdd %v4float %449 %426
        %451 = OpFMul %v4float %450 %136
        %452 = OpFSub %v4float %448 %451
        %453 = OpFMul %v4float %452 %137
        %454 = OpFAdd %v4float %448 %453
        %455 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_1 %int_3
        %456 = OpLoad %float %455
        %457 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_2 %int_3
        %458 = OpLoad %float %457
        %459 = OpFAdd %float %456 %458
        %460 = OpFMul %float %float_0_5 %459
        %461 = OpCompositeInsert %v4float %460 %140 0
        %462 = OpLoad %float %457
        %463 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_0 %int_3
        %464 = OpLoad %float %463
        %465 = OpFAdd %float %462 %464
        %466 = OpFMul %float %float_0_5 %465
        %467 = OpCompositeInsert %v4float %466 %461 1
        %468 = OpLoad %float %463
        %469 = OpLoad %float %455
        %470 = OpFAdd %float %468 %469
        %471 = OpFMul %float %float_0_5 %470
        %472 = OpCompositeInsert %v4float %471 %467 2
        %473 = OpLoad %float %463
        %474 = OpLoad %float %455
        %475 = OpFAdd %float %473 %474
        %476 = OpLoad %float %457
        %477 = OpFAdd %float %475 %476
        %478 = OpFMul %float %float_0_333000004 %477
        %479 = OpCompositeInsert %v4float %478 %472 3
        %480 = OpVectorShuffle %v3float %426 %426 0 1 2
        %481 = OpVectorShuffle %v3float %432 %432 0 1 2
        %482 = OpVectorShuffle %v3float %438 %438 0 1 2
               OpBranch %483
        %483 = OpLabel
               OpLoopMerge %484 %485 None
               OpBranch %486
        %486 = OpLabel
        %487 = OpMatrixTimesVector %v4float %420 %76
        %488 = OpCompositeExtract %float %426 0
        %489 = OpCompositeExtract %float %426 1
        %490 = OpCompositeExtract %float %426 2
        %491 = OpCompositeConstruct %v4float %488 %489 %490 %float_1
        %492 = OpMatrixTimesVector %v4float %418 %491
        %493 = OpVectorShuffle %v3float %492 %492 0 1 2
        %494 = OpVectorShuffle %v3float %487 %487 0 1 2
        %495 = OpFSub %v3float %493 %494
        %496 = OpCompositeExtract %float %492 3
        %497 = OpCompositeExtract %float %487 3
        %498 = OpFAdd %float %496 %497
        %499 = OpCompositeConstruct %v3float %498 %498 %498
        %500 = OpFOrdLessThan %v3bool %495 %499
        %501 = OpSelect %v3int %500 %81 %73
        %502 = OpFAdd %v3float %493 %494
        %503 = OpFNegate %float %496
        %504 = OpFSub %float %503 %497
        %505 = OpCompositeConstruct %v3float %504 %504 %504
        %506 = OpFOrdGreaterThan %v3bool %502 %505
        %507 = OpSelect %v3int %506 %81 %73
        %508 = OpIMul %v3int %82 %507
        %509 = OpIAdd %v3int %501 %508
        %510 = OpCompositeExtract %float %432 0
        %511 = OpCompositeExtract %float %432 1
        %512 = OpCompositeExtract %float %432 2
        %513 = OpCompositeConstruct %v4float %510 %511 %512 %float_1
        %514 = OpMatrixTimesVector %v4float %418 %513
        %515 = OpVectorShuffle %v3float %514 %514 0 1 2
        %516 = OpFSub %v3float %515 %494
        %517 = OpCompositeExtract %float %514 3
        %518 = OpFAdd %float %517 %497
        %519 = OpCompositeConstruct %v3float %518 %518 %518
        %520 = OpFOrdLessThan %v3bool %516 %519
        %521 = OpSelect %v3int %520 %81 %73
        %522 = OpFAdd %v3float %515 %494
        %523 = OpFNegate %float %517
        %524 = OpFSub %float %523 %497
        %525 = OpCompositeConstruct %v3float %524 %524 %524
        %526 = OpFOrdGreaterThan %v3bool %522 %525
        %527 = OpSelect %v3int %526 %81 %73
        %528 = OpIMul %v3int %82 %527
        %529 = OpIAdd %v3int %521 %528
        %530 = OpBitwiseOr %v3int %509 %529
        %531 = OpCompositeExtract %float %438 0
        %532 = OpCompositeExtract %float %438 1
        %533 = OpCompositeExtract %float %438 2
        %534 = OpCompositeConstruct %v4float %531 %532 %533 %float_1
        %535 = OpMatrixTimesVector %v4float %418 %534
        %536 = OpVectorShuffle %v3float %535 %535 0 1 2
        %537 = OpFSub %v3float %536 %494
        %538 = OpCompositeExtract %float %535 3
        %539 = OpFAdd %float %538 %497
        %540 = OpCompositeConstruct %v3float %539 %539 %539
        %541 = OpFOrdLessThan %v3bool %537 %540
        %542 = OpSelect %v3int %541 %81 %73
        %543 = OpFAdd %v3float %536 %494
        %544 = OpFNegate %float %538
        %545 = OpFSub %float %544 %497
        %546 = OpCompositeConstruct %v3float %545 %545 %545
        %547 = OpFOrdGreaterThan %v3bool %543 %546
        %548 = OpSelect %v3int %547 %81 %73
        %549 = OpIMul %v3int %82 %548
        %550 = OpIAdd %v3int %542 %549
        %551 = OpBitwiseOr %v3int %530 %550
        %552 = OpINotEqual %v3bool %551 %74
        %553 = OpAny %bool %552
               OpSelectionMerge %554 None
               OpBranchConditional %553 %555 %554
        %555 = OpLabel
               OpBranch %484
        %554 = OpLabel
        %556 = OpFSub %v3float %480 %481
        %557 = OpFSub %v3float %481 %482
        %558 = OpFSub %v3float %482 %480
        %559 = OpFAdd %v3float %480 %481
        %560 = OpFMul %v3float %77 %559
        %561 = OpFSub %v3float %560 %422
        %562 = OpFAdd %v3float %481 %482
        %563 = OpFMul %v3float %77 %562
        %564 = OpFSub %v3float %563 %422
        %565 = OpFAdd %v3float %482 %480
        %566 = OpFMul %v3float %77 %565
        %567 = OpFSub %v3float %566 %422
        %568 = OpDot %float %557 %557
        %569 = OpDot %float %564 %564
        %570 = OpFDiv %float %568 %569
        %571 = OpExtInst %float %1 Sqrt %570
        %572 = OpDot %float %558 %558
        %573 = OpDot %float %567 %567
        %574 = OpFDiv %float %572 %573
        %575 = OpExtInst %float %1 Sqrt %574
        %576 = OpDot %float %556 %556
        %577 = OpDot %float %561 %561
        %578 = OpFDiv %float %576 %577
        %579 = OpExtInst %float %1 Sqrt %578
        %580 = OpCompositeConstruct %v4float %571 %575 %579 %float_1
        %581 = OpFAdd %float %571 %575
        %582 = OpFAdd %float %581 %579
        %583 = OpFMul %float %float_0_333000004 %582
        %584 = OpCompositeInsert %v4float %583 %580 3
        %585 = OpCompositeConstruct %v4float %424 %424 %424 %424
        %586 = OpFMul %v4float %585 %584
               OpBranch %484
        %485 = OpLabel
               OpBranch %483
        %484 = OpLabel
        %587 = OpPhi %v4float %76 %555 %586 %554
        %588 = OpFMul %v4float %479 %587
        %589 = OpExtInst %v4float %1 FClamp %588 %67 %69
        %590 = OpCompositeExtract %float %589 0
        %591 = OpCompositeExtract %float %589 1
        %592 = OpCompositeExtract %float %589 2
        %593 = OpCompositeExtract %float %589 3
        %594 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_0
               OpStore %594 %590
        %595 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_1
               OpStore %595 %591
        %596 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_2
               OpStore %596 %592
        %597 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0
               OpStore %597 %593
               OpStore %out_var_PN_POSITION9 %454
               OpBranch %if_merge
   %if_merge = OpLabel
               OpReturn
               OpFunctionEnd
