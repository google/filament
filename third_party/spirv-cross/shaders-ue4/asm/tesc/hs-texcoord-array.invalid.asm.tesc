; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 531
; Schema: 0
               OpCapability Tessellation
               OpCapability SampledBuffer
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %MainHull "main" %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_COLOR0 %in_var_TEXCOORD0 %in_var_VS_To_DS_Position %gl_InvocationID %out_var_TEXCOORD10_centroid %out_var_TEXCOORD11_centroid %out_var_COLOR0 %out_var_TEXCOORD0 %out_var_VS_To_DS_Position %out_var_PN_POSITION %out_var_PN_DisplacementScales %out_var_PN_TessellationMultiplier %out_var_PN_WorldDisplacementMultiplier %gl_TessLevelOuter %gl_TessLevelInner %out_var_PN_POSITION9
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
               OpName %FHitProxyVSToDS "FHitProxyVSToDS"
               OpMemberName %FHitProxyVSToDS 0 "FactoryInterpolants"
               OpMemberName %FHitProxyVSToDS 1 "Position"
               OpName %FVertexFactoryInterpolantsVSToDS "FVertexFactoryInterpolantsVSToDS"
               OpMemberName %FVertexFactoryInterpolantsVSToDS 0 "InterpolantsVSToPS"
               OpName %FVertexFactoryInterpolantsVSToPS "FVertexFactoryInterpolantsVSToPS"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 0 "TangentToWorld0"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 1 "TangentToWorld2"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 2 "Color"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 3 "TexCoords"
               OpName %type_View "type.View"
               OpMemberName %type_View 0 "View_TranslatedWorldToClip"
               OpMemberName %type_View 1 "View_WorldToClip"
               OpMemberName %type_View 2 "View_ClipToWorld"
               OpMemberName %type_View 3 "View_TranslatedWorldToView"
               OpMemberName %type_View 4 "View_ViewToTranslatedWorld"
               OpMemberName %type_View 5 "View_TranslatedWorldToCameraView"
               OpMemberName %type_View 6 "View_CameraViewToTranslatedWorld"
               OpMemberName %type_View 7 "View_ViewToClip"
               OpMemberName %type_View 8 "View_ViewToClipNoAA"
               OpMemberName %type_View 9 "View_ClipToView"
               OpMemberName %type_View 10 "View_ClipToTranslatedWorld"
               OpMemberName %type_View 11 "View_SVPositionToTranslatedWorld"
               OpMemberName %type_View 12 "View_ScreenToWorld"
               OpMemberName %type_View 13 "View_ScreenToTranslatedWorld"
               OpMemberName %type_View 14 "View_ViewForward"
               OpMemberName %type_View 15 "PrePadding_View_908"
               OpMemberName %type_View 16 "View_ViewUp"
               OpMemberName %type_View 17 "PrePadding_View_924"
               OpMemberName %type_View 18 "View_ViewRight"
               OpMemberName %type_View 19 "PrePadding_View_940"
               OpMemberName %type_View 20 "View_HMDViewNoRollUp"
               OpMemberName %type_View 21 "PrePadding_View_956"
               OpMemberName %type_View 22 "View_HMDViewNoRollRight"
               OpMemberName %type_View 23 "PrePadding_View_972"
               OpMemberName %type_View 24 "View_InvDeviceZToWorldZTransform"
               OpMemberName %type_View 25 "View_ScreenPositionScaleBias"
               OpMemberName %type_View 26 "View_WorldCameraOrigin"
               OpMemberName %type_View 27 "PrePadding_View_1020"
               OpMemberName %type_View 28 "View_TranslatedWorldCameraOrigin"
               OpMemberName %type_View 29 "PrePadding_View_1036"
               OpMemberName %type_View 30 "View_WorldViewOrigin"
               OpMemberName %type_View 31 "PrePadding_View_1052"
               OpMemberName %type_View 32 "View_PreViewTranslation"
               OpMemberName %type_View 33 "PrePadding_View_1068"
               OpMemberName %type_View 34 "View_PrevProjection"
               OpMemberName %type_View 35 "View_PrevViewProj"
               OpMemberName %type_View 36 "View_PrevViewRotationProj"
               OpMemberName %type_View 37 "View_PrevViewToClip"
               OpMemberName %type_View 38 "View_PrevClipToView"
               OpMemberName %type_View 39 "View_PrevTranslatedWorldToClip"
               OpMemberName %type_View 40 "View_PrevTranslatedWorldToView"
               OpMemberName %type_View 41 "View_PrevViewToTranslatedWorld"
               OpMemberName %type_View 42 "View_PrevTranslatedWorldToCameraView"
               OpMemberName %type_View 43 "View_PrevCameraViewToTranslatedWorld"
               OpMemberName %type_View 44 "View_PrevWorldCameraOrigin"
               OpMemberName %type_View 45 "PrePadding_View_1724"
               OpMemberName %type_View 46 "View_PrevWorldViewOrigin"
               OpMemberName %type_View 47 "PrePadding_View_1740"
               OpMemberName %type_View 48 "View_PrevPreViewTranslation"
               OpMemberName %type_View 49 "PrePadding_View_1756"
               OpMemberName %type_View 50 "View_PrevInvViewProj"
               OpMemberName %type_View 51 "View_PrevScreenToTranslatedWorld"
               OpMemberName %type_View 52 "View_ClipToPrevClip"
               OpMemberName %type_View 53 "View_TemporalAAJitter"
               OpMemberName %type_View 54 "View_GlobalClippingPlane"
               OpMemberName %type_View 55 "View_FieldOfViewWideAngles"
               OpMemberName %type_View 56 "View_PrevFieldOfViewWideAngles"
               OpMemberName %type_View 57 "View_ViewRectMin"
               OpMemberName %type_View 58 "View_ViewSizeAndInvSize"
               OpMemberName %type_View 59 "View_BufferSizeAndInvSize"
               OpMemberName %type_View 60 "View_BufferBilinearUVMinMax"
               OpMemberName %type_View 61 "View_NumSceneColorMSAASamples"
               OpMemberName %type_View 62 "View_PreExposure"
               OpMemberName %type_View 63 "View_OneOverPreExposure"
               OpMemberName %type_View 64 "PrePadding_View_2076"
               OpMemberName %type_View 65 "View_DiffuseOverrideParameter"
               OpMemberName %type_View 66 "View_SpecularOverrideParameter"
               OpMemberName %type_View 67 "View_NormalOverrideParameter"
               OpMemberName %type_View 68 "View_RoughnessOverrideParameter"
               OpMemberName %type_View 69 "View_PrevFrameGameTime"
               OpMemberName %type_View 70 "View_PrevFrameRealTime"
               OpMemberName %type_View 71 "View_OutOfBoundsMask"
               OpMemberName %type_View 72 "PrePadding_View_2148"
               OpMemberName %type_View 73 "PrePadding_View_2152"
               OpMemberName %type_View 74 "PrePadding_View_2156"
               OpMemberName %type_View 75 "View_WorldCameraMovementSinceLastFrame"
               OpMemberName %type_View 76 "View_CullingSign"
               OpMemberName %type_View 77 "View_NearPlane"
               OpMemberName %type_View 78 "View_AdaptiveTessellationFactor"
               OpMemberName %type_View 79 "View_GameTime"
               OpMemberName %type_View 80 "View_RealTime"
               OpMemberName %type_View 81 "View_DeltaTime"
               OpMemberName %type_View 82 "View_MaterialTextureMipBias"
               OpMemberName %type_View 83 "View_MaterialTextureDerivativeMultiply"
               OpMemberName %type_View 84 "View_Random"
               OpMemberName %type_View 85 "View_FrameNumber"
               OpMemberName %type_View 86 "View_StateFrameIndexMod8"
               OpMemberName %type_View 87 "View_StateFrameIndex"
               OpMemberName %type_View 88 "View_CameraCut"
               OpMemberName %type_View 89 "View_UnlitViewmodeMask"
               OpMemberName %type_View 90 "PrePadding_View_2228"
               OpMemberName %type_View 91 "PrePadding_View_2232"
               OpMemberName %type_View 92 "PrePadding_View_2236"
               OpMemberName %type_View 93 "View_DirectionalLightColor"
               OpMemberName %type_View 94 "View_DirectionalLightDirection"
               OpMemberName %type_View 95 "PrePadding_View_2268"
               OpMemberName %type_View 96 "View_TranslucencyLightingVolumeMin"
               OpMemberName %type_View 97 "View_TranslucencyLightingVolumeInvSize"
               OpMemberName %type_View 98 "View_TemporalAAParams"
               OpMemberName %type_View 99 "View_CircleDOFParams"
               OpMemberName %type_View 100 "View_DepthOfFieldSensorWidth"
               OpMemberName %type_View 101 "View_DepthOfFieldFocalDistance"
               OpMemberName %type_View 102 "View_DepthOfFieldScale"
               OpMemberName %type_View 103 "View_DepthOfFieldFocalLength"
               OpMemberName %type_View 104 "View_DepthOfFieldFocalRegion"
               OpMemberName %type_View 105 "View_DepthOfFieldNearTransitionRegion"
               OpMemberName %type_View 106 "View_DepthOfFieldFarTransitionRegion"
               OpMemberName %type_View 107 "View_MotionBlurNormalizedToPixel"
               OpMemberName %type_View 108 "View_bSubsurfacePostprocessEnabled"
               OpMemberName %type_View 109 "View_GeneralPurposeTweak"
               OpMemberName %type_View 110 "View_DemosaicVposOffset"
               OpMemberName %type_View 111 "PrePadding_View_2412"
               OpMemberName %type_View 112 "View_IndirectLightingColorScale"
               OpMemberName %type_View 113 "View_HDR32bppEncodingMode"
               OpMemberName %type_View 114 "View_AtmosphericFogSunDirection"
               OpMemberName %type_View 115 "View_AtmosphericFogSunPower"
               OpMemberName %type_View 116 "View_AtmosphericFogPower"
               OpMemberName %type_View 117 "View_AtmosphericFogDensityScale"
               OpMemberName %type_View 118 "View_AtmosphericFogDensityOffset"
               OpMemberName %type_View 119 "View_AtmosphericFogGroundOffset"
               OpMemberName %type_View 120 "View_AtmosphericFogDistanceScale"
               OpMemberName %type_View 121 "View_AtmosphericFogAltitudeScale"
               OpMemberName %type_View 122 "View_AtmosphericFogHeightScaleRayleigh"
               OpMemberName %type_View 123 "View_AtmosphericFogStartDistance"
               OpMemberName %type_View 124 "View_AtmosphericFogDistanceOffset"
               OpMemberName %type_View 125 "View_AtmosphericFogSunDiscScale"
               OpMemberName %type_View 126 "View_AtmosphericFogSunDiscHalfApexAngleRadian"
               OpMemberName %type_View 127 "PrePadding_View_2492"
               OpMemberName %type_View 128 "View_AtmosphericFogSunDiscLuminance"
               OpMemberName %type_View 129 "View_AtmosphericFogRenderMask"
               OpMemberName %type_View 130 "View_AtmosphericFogInscatterAltitudeSampleNum"
               OpMemberName %type_View 131 "PrePadding_View_2520"
               OpMemberName %type_View 132 "PrePadding_View_2524"
               OpMemberName %type_View 133 "View_AtmosphericFogSunColor"
               OpMemberName %type_View 134 "View_NormalCurvatureToRoughnessScaleBias"
               OpMemberName %type_View 135 "View_RenderingReflectionCaptureMask"
               OpMemberName %type_View 136 "View_AmbientCubemapTint"
               OpMemberName %type_View 137 "View_AmbientCubemapIntensity"
               OpMemberName %type_View 138 "View_SkyLightParameters"
               OpMemberName %type_View 139 "PrePadding_View_2584"
               OpMemberName %type_View 140 "PrePadding_View_2588"
               OpMemberName %type_View 141 "View_SkyLightColor"
               OpMemberName %type_View 142 "View_SkyIrradianceEnvironmentMap"
               OpMemberName %type_View 143 "View_MobilePreviewMode"
               OpMemberName %type_View 144 "View_HMDEyePaddingOffset"
               OpMemberName %type_View 145 "View_ReflectionCubemapMaxMip"
               OpMemberName %type_View 146 "View_ShowDecalsMask"
               OpMemberName %type_View 147 "View_DistanceFieldAOSpecularOcclusionMode"
               OpMemberName %type_View 148 "View_IndirectCapsuleSelfShadowingIntensity"
               OpMemberName %type_View 149 "PrePadding_View_2744"
               OpMemberName %type_View 150 "PrePadding_View_2748"
               OpMemberName %type_View 151 "View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight"
               OpMemberName %type_View 152 "View_StereoPassIndex"
               OpMemberName %type_View 153 "View_GlobalVolumeCenterAndExtent"
               OpMemberName %type_View 154 "View_GlobalVolumeWorldToUVAddAndMul"
               OpMemberName %type_View 155 "View_GlobalVolumeDimension"
               OpMemberName %type_View 156 "View_GlobalVolumeTexelSize"
               OpMemberName %type_View 157 "View_MaxGlobalDistance"
               OpMemberName %type_View 158 "PrePadding_View_2908"
               OpMemberName %type_View 159 "View_CursorPosition"
               OpMemberName %type_View 160 "View_bCheckerboardSubsurfaceProfileRendering"
               OpMemberName %type_View 161 "PrePadding_View_2924"
               OpMemberName %type_View 162 "View_VolumetricFogInvGridSize"
               OpMemberName %type_View 163 "PrePadding_View_2940"
               OpMemberName %type_View 164 "View_VolumetricFogGridZParams"
               OpMemberName %type_View 165 "PrePadding_View_2956"
               OpMemberName %type_View 166 "View_VolumetricFogSVPosToVolumeUV"
               OpMemberName %type_View 167 "View_VolumetricFogMaxDistance"
               OpMemberName %type_View 168 "PrePadding_View_2972"
               OpMemberName %type_View 169 "View_VolumetricLightmapWorldToUVScale"
               OpMemberName %type_View 170 "PrePadding_View_2988"
               OpMemberName %type_View 171 "View_VolumetricLightmapWorldToUVAdd"
               OpMemberName %type_View 172 "PrePadding_View_3004"
               OpMemberName %type_View 173 "View_VolumetricLightmapIndirectionTextureSize"
               OpMemberName %type_View 174 "View_VolumetricLightmapBrickSize"
               OpMemberName %type_View 175 "View_VolumetricLightmapBrickTexelSize"
               OpMemberName %type_View 176 "View_StereoIPD"
               OpMemberName %type_View 177 "View_IndirectLightingCacheShowFlag"
               OpMemberName %type_View 178 "View_EyeToPixelSpreadAngle"
               OpName %View "View"
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
               OpName %in_var_TEXCOORD10_centroid "in.var.TEXCOORD10_centroid"
               OpName %in_var_TEXCOORD11_centroid "in.var.TEXCOORD11_centroid"
               OpName %in_var_COLOR0 "in.var.COLOR0"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %in_var_VS_To_DS_Position "in.var.VS_To_DS_Position"
               OpName %out_var_TEXCOORD10_centroid "out.var.TEXCOORD10_centroid"
               OpName %out_var_TEXCOORD11_centroid "out.var.TEXCOORD11_centroid"
               OpName %out_var_COLOR0 "out.var.COLOR0"
               OpName %out_var_TEXCOORD0 "out.var.TEXCOORD0"
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
               OpDecorateString %in_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorateString %gl_InvocationID UserSemantic "SV_OutputControlPointID"
               OpDecorateString %out_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %out_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %out_var_COLOR0 UserSemantic "COLOR0"
               OpDecorateString %out_var_TEXCOORD0 UserSemantic "TEXCOORD0"
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
               OpDecorate %in_var_VS_To_DS_Position Location 5
               OpDecorate %out_var_COLOR0 Location 0
               OpDecorate %out_var_PN_DisplacementScales Location 1
               OpDecorate %out_var_PN_POSITION Location 2
               OpDecorate %out_var_PN_POSITION9 Location 5
               OpDecorate %out_var_PN_TessellationMultiplier Location 6
               OpDecorate %out_var_PN_WorldDisplacementMultiplier Location 7
               OpDecorate %out_var_TEXCOORD0 Location 8
               OpDecorate %out_var_TEXCOORD10_centroid Location 10
               OpDecorate %out_var_TEXCOORD11_centroid Location 11
               OpDecorate %out_var_VS_To_DS_Position Location 12
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 0
               OpDecorate %Primitive DescriptorSet 0
               OpDecorate %Primitive Binding 1
               OpDecorate %_arr_v4float_uint_2 ArrayStride 16
               OpDecorate %_arr_v4float_uint_7 ArrayStride 16
               OpDecorate %_arr_v4float_uint_4 ArrayStride 16
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
               OpMemberDecorate %type_View 13 MatrixStride 16
               OpMemberDecorate %type_View 13 ColMajor
               OpMemberDecorate %type_View 14 Offset 896
               OpMemberDecorate %type_View 15 Offset 908
               OpMemberDecorate %type_View 16 Offset 912
               OpMemberDecorate %type_View 17 Offset 924
               OpMemberDecorate %type_View 18 Offset 928
               OpMemberDecorate %type_View 19 Offset 940
               OpMemberDecorate %type_View 20 Offset 944
               OpMemberDecorate %type_View 21 Offset 956
               OpMemberDecorate %type_View 22 Offset 960
               OpMemberDecorate %type_View 23 Offset 972
               OpMemberDecorate %type_View 24 Offset 976
               OpMemberDecorate %type_View 25 Offset 992
               OpMemberDecorate %type_View 26 Offset 1008
               OpMemberDecorate %type_View 27 Offset 1020
               OpMemberDecorate %type_View 28 Offset 1024
               OpMemberDecorate %type_View 29 Offset 1036
               OpMemberDecorate %type_View 30 Offset 1040
               OpMemberDecorate %type_View 31 Offset 1052
               OpMemberDecorate %type_View 32 Offset 1056
               OpMemberDecorate %type_View 33 Offset 1068
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
               OpMemberDecorate %type_View 43 MatrixStride 16
               OpMemberDecorate %type_View 43 ColMajor
               OpMemberDecorate %type_View 44 Offset 1712
               OpMemberDecorate %type_View 45 Offset 1724
               OpMemberDecorate %type_View 46 Offset 1728
               OpMemberDecorate %type_View 47 Offset 1740
               OpMemberDecorate %type_View 48 Offset 1744
               OpMemberDecorate %type_View 49 Offset 1756
               OpMemberDecorate %type_View 50 Offset 1760
               OpMemberDecorate %type_View 50 MatrixStride 16
               OpMemberDecorate %type_View 50 ColMajor
               OpMemberDecorate %type_View 51 Offset 1824
               OpMemberDecorate %type_View 51 MatrixStride 16
               OpMemberDecorate %type_View 51 ColMajor
               OpMemberDecorate %type_View 52 Offset 1888
               OpMemberDecorate %type_View 52 MatrixStride 16
               OpMemberDecorate %type_View 52 ColMajor
               OpMemberDecorate %type_View 53 Offset 1952
               OpMemberDecorate %type_View 54 Offset 1968
               OpMemberDecorate %type_View 55 Offset 1984
               OpMemberDecorate %type_View 56 Offset 1992
               OpMemberDecorate %type_View 57 Offset 2000
               OpMemberDecorate %type_View 58 Offset 2016
               OpMemberDecorate %type_View 59 Offset 2032
               OpMemberDecorate %type_View 60 Offset 2048
               OpMemberDecorate %type_View 61 Offset 2064
               OpMemberDecorate %type_View 62 Offset 2068
               OpMemberDecorate %type_View 63 Offset 2072
               OpMemberDecorate %type_View 64 Offset 2076
               OpMemberDecorate %type_View 65 Offset 2080
               OpMemberDecorate %type_View 66 Offset 2096
               OpMemberDecorate %type_View 67 Offset 2112
               OpMemberDecorate %type_View 68 Offset 2128
               OpMemberDecorate %type_View 69 Offset 2136
               OpMemberDecorate %type_View 70 Offset 2140
               OpMemberDecorate %type_View 71 Offset 2144
               OpMemberDecorate %type_View 72 Offset 2148
               OpMemberDecorate %type_View 73 Offset 2152
               OpMemberDecorate %type_View 74 Offset 2156
               OpMemberDecorate %type_View 75 Offset 2160
               OpMemberDecorate %type_View 76 Offset 2172
               OpMemberDecorate %type_View 77 Offset 2176
               OpMemberDecorate %type_View 78 Offset 2180
               OpMemberDecorate %type_View 79 Offset 2184
               OpMemberDecorate %type_View 80 Offset 2188
               OpMemberDecorate %type_View 81 Offset 2192
               OpMemberDecorate %type_View 82 Offset 2196
               OpMemberDecorate %type_View 83 Offset 2200
               OpMemberDecorate %type_View 84 Offset 2204
               OpMemberDecorate %type_View 85 Offset 2208
               OpMemberDecorate %type_View 86 Offset 2212
               OpMemberDecorate %type_View 87 Offset 2216
               OpMemberDecorate %type_View 88 Offset 2220
               OpMemberDecorate %type_View 89 Offset 2224
               OpMemberDecorate %type_View 90 Offset 2228
               OpMemberDecorate %type_View 91 Offset 2232
               OpMemberDecorate %type_View 92 Offset 2236
               OpMemberDecorate %type_View 93 Offset 2240
               OpMemberDecorate %type_View 94 Offset 2256
               OpMemberDecorate %type_View 95 Offset 2268
               OpMemberDecorate %type_View 96 Offset 2272
               OpMemberDecorate %type_View 97 Offset 2304
               OpMemberDecorate %type_View 98 Offset 2336
               OpMemberDecorate %type_View 99 Offset 2352
               OpMemberDecorate %type_View 100 Offset 2368
               OpMemberDecorate %type_View 101 Offset 2372
               OpMemberDecorate %type_View 102 Offset 2376
               OpMemberDecorate %type_View 103 Offset 2380
               OpMemberDecorate %type_View 104 Offset 2384
               OpMemberDecorate %type_View 105 Offset 2388
               OpMemberDecorate %type_View 106 Offset 2392
               OpMemberDecorate %type_View 107 Offset 2396
               OpMemberDecorate %type_View 108 Offset 2400
               OpMemberDecorate %type_View 109 Offset 2404
               OpMemberDecorate %type_View 110 Offset 2408
               OpMemberDecorate %type_View 111 Offset 2412
               OpMemberDecorate %type_View 112 Offset 2416
               OpMemberDecorate %type_View 113 Offset 2428
               OpMemberDecorate %type_View 114 Offset 2432
               OpMemberDecorate %type_View 115 Offset 2444
               OpMemberDecorate %type_View 116 Offset 2448
               OpMemberDecorate %type_View 117 Offset 2452
               OpMemberDecorate %type_View 118 Offset 2456
               OpMemberDecorate %type_View 119 Offset 2460
               OpMemberDecorate %type_View 120 Offset 2464
               OpMemberDecorate %type_View 121 Offset 2468
               OpMemberDecorate %type_View 122 Offset 2472
               OpMemberDecorate %type_View 123 Offset 2476
               OpMemberDecorate %type_View 124 Offset 2480
               OpMemberDecorate %type_View 125 Offset 2484
               OpMemberDecorate %type_View 126 Offset 2488
               OpMemberDecorate %type_View 127 Offset 2492
               OpMemberDecorate %type_View 128 Offset 2496
               OpMemberDecorate %type_View 129 Offset 2512
               OpMemberDecorate %type_View 130 Offset 2516
               OpMemberDecorate %type_View 131 Offset 2520
               OpMemberDecorate %type_View 132 Offset 2524
               OpMemberDecorate %type_View 133 Offset 2528
               OpMemberDecorate %type_View 134 Offset 2544
               OpMemberDecorate %type_View 135 Offset 2556
               OpMemberDecorate %type_View 136 Offset 2560
               OpMemberDecorate %type_View 137 Offset 2576
               OpMemberDecorate %type_View 138 Offset 2580
               OpMemberDecorate %type_View 139 Offset 2584
               OpMemberDecorate %type_View 140 Offset 2588
               OpMemberDecorate %type_View 141 Offset 2592
               OpMemberDecorate %type_View 142 Offset 2608
               OpMemberDecorate %type_View 143 Offset 2720
               OpMemberDecorate %type_View 144 Offset 2724
               OpMemberDecorate %type_View 145 Offset 2728
               OpMemberDecorate %type_View 146 Offset 2732
               OpMemberDecorate %type_View 147 Offset 2736
               OpMemberDecorate %type_View 148 Offset 2740
               OpMemberDecorate %type_View 149 Offset 2744
               OpMemberDecorate %type_View 150 Offset 2748
               OpMemberDecorate %type_View 151 Offset 2752
               OpMemberDecorate %type_View 152 Offset 2764
               OpMemberDecorate %type_View 153 Offset 2768
               OpMemberDecorate %type_View 154 Offset 2832
               OpMemberDecorate %type_View 155 Offset 2896
               OpMemberDecorate %type_View 156 Offset 2900
               OpMemberDecorate %type_View 157 Offset 2904
               OpMemberDecorate %type_View 158 Offset 2908
               OpMemberDecorate %type_View 159 Offset 2912
               OpMemberDecorate %type_View 160 Offset 2920
               OpMemberDecorate %type_View 161 Offset 2924
               OpMemberDecorate %type_View 162 Offset 2928
               OpMemberDecorate %type_View 163 Offset 2940
               OpMemberDecorate %type_View 164 Offset 2944
               OpMemberDecorate %type_View 165 Offset 2956
               OpMemberDecorate %type_View 166 Offset 2960
               OpMemberDecorate %type_View 167 Offset 2968
               OpMemberDecorate %type_View 168 Offset 2972
               OpMemberDecorate %type_View 169 Offset 2976
               OpMemberDecorate %type_View 170 Offset 2988
               OpMemberDecorate %type_View 171 Offset 2992
               OpMemberDecorate %type_View 172 Offset 3004
               OpMemberDecorate %type_View 173 Offset 3008
               OpMemberDecorate %type_View 174 Offset 3020
               OpMemberDecorate %type_View 175 Offset 3024
               OpMemberDecorate %type_View 176 Offset 3036
               OpMemberDecorate %type_View 177 Offset 3040
               OpMemberDecorate %type_View 178 Offset 3044
               OpDecorate %type_View Block
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
      %v2int = OpTypeVector %int 2
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
%_arr_float_uint_3 = OpTypeArray %float %uint_3
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
    %float_2 = OpConstant %float 2
         %54 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
  %float_0_5 = OpConstant %float 0.5
      %int_3 = OpConstant %int 3
%float_0_333000004 = OpConstant %float 0.333000004
    %float_1 = OpConstant %float 1
         %59 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
   %float_15 = OpConstant %float 15
         %61 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%_arr_v2float_uint_2 = OpTypeArray %v2float %uint_2
%FVertexFactoryInterpolantsVSToPS = OpTypeStruct %v4float %v4float %v4float %_arr_v2float_uint_2
%FVertexFactoryInterpolantsVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToPS
%FHitProxyVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToDS %v4float
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%FPNTessellationHSToDS = OpTypeStruct %FHitProxyVSToDS %_arr_v4float_uint_3 %v3float %float %float
      %v3int = OpTypeVector %int 3
         %65 = OpConstantComposite %v3int %int_0 %int_0 %int_0
         %66 = OpConstantComposite %v3int %int_3 %int_3 %int_3
    %float_0 = OpConstant %float 0
         %68 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %69 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %int_78 = OpConstant %int 78
     %int_15 = OpConstant %int 15
      %int_7 = OpConstant %int 7
     %int_28 = OpConstant %int 28
         %74 = OpConstantComposite %v3int %int_1 %int_1 %int_1
         %75 = OpConstantComposite %v3int %int_2 %int_2 %int_2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %float %float %v4float %uint %uint %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v2int %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%type_Primitive = OpTypeStruct %mat4v4float %v4float %v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %float %float %float %float %v4float %v4float %v3float %uint %v3float %uint %v3float %int %uint %uint %uint %uint %_arr_v4float_uint_4
%_ptr_Uniform_type_Primitive = OpTypePointer Uniform %type_Primitive
    %uint_12 = OpConstant %uint 12
%_arr_v4float_uint_12 = OpTypeArray %v4float %uint_12
%_ptr_Input__arr_v4float_uint_12 = OpTypePointer Input %_arr_v4float_uint_12
%_arr__arr_v2float_uint_2_uint_12 = OpTypeArray %_arr_v2float_uint_2 %uint_12
%_ptr_Input__arr__arr_v2float_uint_2_uint_12 = OpTypePointer Input %_arr__arr_v2float_uint_2_uint_12
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
%_arr__arr_v2float_uint_2_uint_3 = OpTypeArray %_arr_v2float_uint_2 %uint_3
%_ptr_Output__arr__arr_v2float_uint_2_uint_3 = OpTypePointer Output %_arr__arr_v2float_uint_2_uint_3
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
         %98 = OpTypeFunction %void
%_arr_FHitProxyVSToDS_uint_12 = OpTypeArray %FHitProxyVSToDS %uint_12
%_ptr_Function__arr_FHitProxyVSToDS_uint_12 = OpTypePointer Function %_arr_FHitProxyVSToDS_uint_12
%_arr_FPNTessellationHSToDS_uint_3 = OpTypeArray %FPNTessellationHSToDS %uint_3
%_ptr_Function__arr_FPNTessellationHSToDS_uint_3 = OpTypePointer Function %_arr_FPNTessellationHSToDS_uint_3
%_ptr_Workgroup__arr_FPNTessellationHSToDS_uint_3 = OpTypePointer Workgroup %_arr_FPNTessellationHSToDS_uint_3
%_ptr_Output__arr_v2float_uint_2 = OpTypePointer Output %_arr_v2float_uint_2
%_ptr_Output_v3float = OpTypePointer Output %v3float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Function_FPNTessellationHSToDS = OpTypePointer Function %FPNTessellationHSToDS
%_ptr_Workgroup_FPNTessellationHSToDS = OpTypePointer Workgroup %FPNTessellationHSToDS
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Workgroup_v4float = OpTypePointer Workgroup %v4float
%_ptr_Workgroup_float = OpTypePointer Workgroup %float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_FVertexFactoryInterpolantsVSToDS = OpTypePointer Function %FVertexFactoryInterpolantsVSToDS
%_ptr_Function_FHitProxyVSToDS = OpTypePointer Function %FHitProxyVSToDS
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
       %View = OpVariable %_ptr_Uniform_type_View Uniform
  %Primitive = OpVariable %_ptr_Uniform_type_Primitive Uniform
%in_var_TEXCOORD10_centroid = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_TEXCOORD11_centroid = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_COLOR0 = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_TEXCOORD0 = OpVariable %_ptr_Input__arr__arr_v2float_uint_2_uint_12 Input
%in_var_VS_To_DS_Position = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%gl_InvocationID = OpVariable %_ptr_Input_uint Input
%out_var_TEXCOORD10_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_TEXCOORD11_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_COLOR0 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_TEXCOORD0 = OpVariable %_ptr_Output__arr__arr_v2float_uint_2_uint_3 Output
%out_var_VS_To_DS_Position = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_PN_POSITION = OpVariable %_ptr_Output__arr__arr_v4float_uint_3_uint_3 Output
%out_var_PN_DisplacementScales = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%out_var_PN_TessellationMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%out_var_PN_WorldDisplacementMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
%out_var_PN_POSITION9 = OpVariable %_ptr_Output_v4float Output
%float_0_333333343 = OpConstant %float 0.333333343
        %119 = OpConstantComposite %v4float %float_0_333333343 %float_0_333333343 %float_0_333333343 %float_0_333333343
        %120 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%float_0_166666672 = OpConstant %float 0.166666672
        %122 = OpConstantComposite %v4float %float_0_166666672 %float_0_166666672 %float_0_166666672 %float_0_166666672
        %123 = OpUndef %v4float

; XXX: Original asm used Function here, which is wrong.
; This patches the SPIR-V to be correct.
%temp_var_hullMainRetVal = OpVariable %_ptr_Workgroup__arr_FPNTessellationHSToDS_uint_3 Workgroup

   %MainHull = OpFunction %void None %98
        %124 = OpLabel
%param_var_I = OpVariable %_ptr_Function__arr_FHitProxyVSToDS_uint_12 Function
        %125 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD10_centroid
        %126 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD11_centroid
        %127 = OpLoad %_arr_v4float_uint_12 %in_var_COLOR0
        %128 = OpLoad %_arr__arr_v2float_uint_2_uint_12 %in_var_TEXCOORD0
        %129 = OpCompositeExtract %v4float %125 0
        %130 = OpCompositeExtract %v4float %126 0
        %131 = OpCompositeExtract %v4float %127 0
        %132 = OpCompositeExtract %_arr_v2float_uint_2 %128 0
        %133 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %129 %130 %131 %132
        %134 = OpCompositeExtract %v4float %125 1
        %135 = OpCompositeExtract %v4float %126 1
        %136 = OpCompositeExtract %v4float %127 1
        %137 = OpCompositeExtract %_arr_v2float_uint_2 %128 1
        %138 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %134 %135 %136 %137
        %139 = OpCompositeExtract %v4float %125 2
        %140 = OpCompositeExtract %v4float %126 2
        %141 = OpCompositeExtract %v4float %127 2
        %142 = OpCompositeExtract %_arr_v2float_uint_2 %128 2
        %143 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %139 %140 %141 %142
        %144 = OpCompositeExtract %v4float %125 3
        %145 = OpCompositeExtract %v4float %126 3
        %146 = OpCompositeExtract %v4float %127 3
        %147 = OpCompositeExtract %_arr_v2float_uint_2 %128 3
        %148 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %144 %145 %146 %147
        %149 = OpCompositeExtract %v4float %125 4
        %150 = OpCompositeExtract %v4float %126 4
        %151 = OpCompositeExtract %v4float %127 4
        %152 = OpCompositeExtract %_arr_v2float_uint_2 %128 4
        %153 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %149 %150 %151 %152
        %154 = OpCompositeExtract %v4float %125 5
        %155 = OpCompositeExtract %v4float %126 5
        %156 = OpCompositeExtract %v4float %127 5
        %157 = OpCompositeExtract %_arr_v2float_uint_2 %128 5
        %158 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %154 %155 %156 %157
        %159 = OpCompositeExtract %v4float %125 6
        %160 = OpCompositeExtract %v4float %126 6
        %161 = OpCompositeExtract %v4float %127 6
        %162 = OpCompositeExtract %_arr_v2float_uint_2 %128 6
        %163 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %159 %160 %161 %162
        %164 = OpCompositeExtract %v4float %125 7
        %165 = OpCompositeExtract %v4float %126 7
        %166 = OpCompositeExtract %v4float %127 7
        %167 = OpCompositeExtract %_arr_v2float_uint_2 %128 7
        %168 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %164 %165 %166 %167
        %169 = OpCompositeExtract %v4float %125 8
        %170 = OpCompositeExtract %v4float %126 8
        %171 = OpCompositeExtract %v4float %127 8
        %172 = OpCompositeExtract %_arr_v2float_uint_2 %128 8
        %173 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %169 %170 %171 %172
        %174 = OpCompositeExtract %v4float %125 9
        %175 = OpCompositeExtract %v4float %126 9
        %176 = OpCompositeExtract %v4float %127 9
        %177 = OpCompositeExtract %_arr_v2float_uint_2 %128 9
        %178 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %174 %175 %176 %177
        %179 = OpCompositeExtract %v4float %125 10
        %180 = OpCompositeExtract %v4float %126 10
        %181 = OpCompositeExtract %v4float %127 10
        %182 = OpCompositeExtract %_arr_v2float_uint_2 %128 10
        %183 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %179 %180 %181 %182
        %184 = OpCompositeExtract %v4float %125 11
        %185 = OpCompositeExtract %v4float %126 11
        %186 = OpCompositeExtract %v4float %127 11
        %187 = OpCompositeExtract %_arr_v2float_uint_2 %128 11
        %188 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %184 %185 %186 %187
        %189 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %133
        %190 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %138
        %191 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %143
        %192 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %148
        %193 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %153
        %194 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %158
        %195 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %163
        %196 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %168
        %197 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %173
        %198 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %178
        %199 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %183
        %200 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %188
        %201 = OpLoad %_arr_v4float_uint_12 %in_var_VS_To_DS_Position
        %202 = OpCompositeExtract %v4float %201 0
        %203 = OpCompositeConstruct %FHitProxyVSToDS %189 %202
        %204 = OpCompositeExtract %v4float %201 1
        %205 = OpCompositeConstruct %FHitProxyVSToDS %190 %204
        %206 = OpCompositeExtract %v4float %201 2
        %207 = OpCompositeConstruct %FHitProxyVSToDS %191 %206
        %208 = OpCompositeExtract %v4float %201 3
        %209 = OpCompositeConstruct %FHitProxyVSToDS %192 %208
        %210 = OpCompositeExtract %v4float %201 4
        %211 = OpCompositeConstruct %FHitProxyVSToDS %193 %210
        %212 = OpCompositeExtract %v4float %201 5
        %213 = OpCompositeConstruct %FHitProxyVSToDS %194 %212
        %214 = OpCompositeExtract %v4float %201 6
        %215 = OpCompositeConstruct %FHitProxyVSToDS %195 %214
        %216 = OpCompositeExtract %v4float %201 7
        %217 = OpCompositeConstruct %FHitProxyVSToDS %196 %216
        %218 = OpCompositeExtract %v4float %201 8
        %219 = OpCompositeConstruct %FHitProxyVSToDS %197 %218
        %220 = OpCompositeExtract %v4float %201 9
        %221 = OpCompositeConstruct %FHitProxyVSToDS %198 %220
        %222 = OpCompositeExtract %v4float %201 10
        %223 = OpCompositeConstruct %FHitProxyVSToDS %199 %222
        %224 = OpCompositeExtract %v4float %201 11
        %225 = OpCompositeConstruct %FHitProxyVSToDS %200 %224
        %226 = OpCompositeConstruct %_arr_FHitProxyVSToDS_uint_12 %203 %205 %207 %209 %211 %213 %215 %217 %219 %221 %223 %225
               OpStore %param_var_I %226
        %227 = OpLoad %uint %gl_InvocationID
        %228 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %227 %int_0
        %229 = OpLoad %FVertexFactoryInterpolantsVSToDS %228
        %230 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %229 0
        %231 = OpCompositeExtract %v4float %230 0
        %232 = OpCompositeExtract %v4float %230 1
        %233 = OpVectorShuffle %v3float %231 %231 0 1 2
        %234 = OpVectorShuffle %v3float %232 %232 0 1 2
        %235 = OpExtInst %v3float %1 Cross %234 %233
        %236 = OpCompositeExtract %float %232 3
        %237 = OpCompositeConstruct %v3float %236 %236 %236
        %238 = OpFMul %v3float %235 %237
        %239 = OpCompositeConstruct %mat3v3float %233 %238 %234
        %240 = OpCompositeExtract %float %232 0
        %241 = OpCompositeExtract %float %232 1
        %242 = OpCompositeExtract %float %232 2
        %243 = OpCompositeConstruct %v4float %240 %241 %242 %float_0
        %244 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_15
        %245 = OpLoad %v4float %244
        %246 = OpVectorShuffle %v3float %245 %245 0 1 2
        %247 = OpVectorTimesMatrix %v3float %246 %239
        %248 = OpULessThan %bool %227 %uint_2
        %249 = OpIAdd %uint %227 %uint_1
        %250 = OpSelect %uint %248 %249 %uint_0
        %251 = OpIMul %uint %uint_2 %227
        %252 = OpIAdd %uint %uint_3 %251
        %253 = OpIAdd %uint %251 %uint_4
        %254 = OpAccessChain %_ptr_Function_FHitProxyVSToDS %param_var_I %227
        %255 = OpLoad %FHitProxyVSToDS %254
        %256 = OpAccessChain %_ptr_Function_v4float %param_var_I %227 %int_1
        %257 = OpLoad %v4float %256
        %258 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %250 %int_0
        %259 = OpLoad %FVertexFactoryInterpolantsVSToDS %258
        %260 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %259 0
        %261 = OpCompositeExtract %v4float %260 1
        %262 = OpCompositeExtract %float %261 0
        %263 = OpCompositeExtract %float %261 1
        %264 = OpCompositeExtract %float %261 2
        %265 = OpCompositeConstruct %v4float %262 %263 %264 %float_0
        %266 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %252 %int_0
        %267 = OpLoad %FVertexFactoryInterpolantsVSToDS %266
        %268 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %267 0
        %269 = OpCompositeExtract %v4float %268 1
        %270 = OpCompositeExtract %float %269 0
        %271 = OpCompositeExtract %float %269 1
        %272 = OpCompositeExtract %float %269 2
        %273 = OpCompositeConstruct %v4float %270 %271 %272 %float_0
        %274 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %253 %int_0
        %275 = OpLoad %FVertexFactoryInterpolantsVSToDS %274
        %276 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %275 0
        %277 = OpCompositeExtract %v4float %276 1
        %278 = OpCompositeExtract %float %277 0
        %279 = OpCompositeExtract %float %277 1
        %280 = OpCompositeExtract %float %277 2
        %281 = OpCompositeConstruct %v4float %278 %279 %280 %float_0
        %282 = OpLoad %v4float %256
        %283 = OpAccessChain %_ptr_Function_v4float %param_var_I %250 %int_1
        %284 = OpLoad %v4float %283
        %285 = OpFMul %v4float %54 %282
        %286 = OpFAdd %v4float %285 %284
        %287 = OpFSub %v4float %284 %282
        %288 = OpDot %float %287 %243
        %289 = OpCompositeConstruct %v4float %288 %288 %288 %288
        %290 = OpFMul %v4float %289 %243
        %291 = OpFSub %v4float %286 %290
        %292 = OpFMul %v4float %291 %119
        %293 = OpAccessChain %_ptr_Function_v4float %param_var_I %252 %int_1
        %294 = OpLoad %v4float %293
        %295 = OpAccessChain %_ptr_Function_v4float %param_var_I %253 %int_1
        %296 = OpLoad %v4float %295
        %297 = OpFMul %v4float %54 %294
        %298 = OpFAdd %v4float %297 %296
        %299 = OpFSub %v4float %296 %294
        %300 = OpDot %float %299 %273
        %301 = OpCompositeConstruct %v4float %300 %300 %300 %300
        %302 = OpFMul %v4float %301 %273
        %303 = OpFSub %v4float %298 %302
        %304 = OpFMul %v4float %303 %119
        %305 = OpFAdd %v4float %292 %304
        %306 = OpFMul %v4float %305 %120
        %307 = OpLoad %v4float %283
        %308 = OpLoad %v4float %256
        %309 = OpFMul %v4float %54 %307
        %310 = OpFAdd %v4float %309 %308
        %311 = OpFSub %v4float %308 %307
        %312 = OpDot %float %311 %265
        %313 = OpCompositeConstruct %v4float %312 %312 %312 %312
        %314 = OpFMul %v4float %313 %265
        %315 = OpFSub %v4float %310 %314
        %316 = OpFMul %v4float %315 %119
        %317 = OpLoad %v4float %295
        %318 = OpLoad %v4float %293
        %319 = OpFMul %v4float %54 %317
        %320 = OpFAdd %v4float %319 %318
        %321 = OpFSub %v4float %318 %317
        %322 = OpDot %float %321 %281
        %323 = OpCompositeConstruct %v4float %322 %322 %322 %322
        %324 = OpFMul %v4float %323 %281
        %325 = OpFSub %v4float %320 %324
        %326 = OpFMul %v4float %325 %119
        %327 = OpFAdd %v4float %316 %326
        %328 = OpFMul %v4float %327 %120
        %329 = OpCompositeConstruct %_arr_v4float_uint_3 %257 %306 %328
        %330 = OpCompositeConstruct %FPNTessellationHSToDS %255 %329 %247 %float_1 %float_1
        %331 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %255 0
        %332 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %331 0
        %333 = OpCompositeExtract %v4float %332 0
        %334 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD10_centroid %227
               OpStore %334 %333
        %335 = OpCompositeExtract %v4float %332 1
        %336 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD11_centroid %227
               OpStore %336 %335
        %337 = OpCompositeExtract %v4float %332 2
        %338 = OpAccessChain %_ptr_Output_v4float %out_var_COLOR0 %227
               OpStore %338 %337
        %339 = OpCompositeExtract %_arr_v2float_uint_2 %332 3
        %340 = OpAccessChain %_ptr_Output__arr_v2float_uint_2 %out_var_TEXCOORD0 %227
               OpStore %340 %339
        %341 = OpCompositeExtract %v4float %255 1
        %342 = OpAccessChain %_ptr_Output_v4float %out_var_VS_To_DS_Position %227
               OpStore %342 %341
        %343 = OpAccessChain %_ptr_Output__arr_v4float_uint_3 %out_var_PN_POSITION %227
               OpStore %343 %329
        %344 = OpAccessChain %_ptr_Output_v3float %out_var_PN_DisplacementScales %227
               OpStore %344 %247
        %345 = OpAccessChain %_ptr_Output_float %out_var_PN_TessellationMultiplier %227
               OpStore %345 %float_1
        %346 = OpAccessChain %_ptr_Output_float %out_var_PN_WorldDisplacementMultiplier %227
               OpStore %346 %float_1
        %347 = OpAccessChain %_ptr_Workgroup_FPNTessellationHSToDS %temp_var_hullMainRetVal %227
               OpStore %347 %330
               OpControlBarrier %uint_2 %uint_4 %uint_0
        %348 = OpIEqual %bool %227 %uint_0
               OpSelectionMerge %if_merge None
               OpBranchConditional %348 %349 %if_merge
        %349 = OpLabel
        %350 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_0
        %351 = OpLoad %mat4v4float %350
        %352 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_7
        %353 = OpLoad %mat4v4float %352
        %354 = OpAccessChain %_ptr_Uniform_v3float %View %int_28
        %355 = OpLoad %v3float %354
        %356 = OpAccessChain %_ptr_Uniform_float %View %int_78
        %357 = OpLoad %float %356
        %358 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_0
        %359 = OpLoad %v4float %358
        %360 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_1
        %361 = OpLoad %v4float %360
        %362 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_2
        %363 = OpLoad %v4float %362
        %364 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_0
        %365 = OpLoad %v4float %364
        %366 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_1
        %367 = OpLoad %v4float %366
        %368 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_2
        %369 = OpLoad %v4float %368
        %370 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_0
        %371 = OpLoad %v4float %370
        %372 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_1
        %373 = OpLoad %v4float %372
        %374 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_2
        %375 = OpLoad %v4float %374
        %376 = OpFAdd %v4float %361 %363
        %377 = OpFAdd %v4float %376 %367
        %378 = OpFAdd %v4float %377 %369
        %379 = OpFAdd %v4float %378 %373
        %380 = OpFAdd %v4float %379 %375
        %381 = OpFMul %v4float %380 %122
        %382 = OpFAdd %v4float %371 %365
        %383 = OpFAdd %v4float %382 %359
        %384 = OpFMul %v4float %383 %119
        %385 = OpFSub %v4float %381 %384
        %386 = OpFMul %v4float %385 %120
        %387 = OpFAdd %v4float %381 %386
        %388 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_1 %int_3
        %389 = OpLoad %float %388
        %390 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_2 %int_3
        %391 = OpLoad %float %390
        %392 = OpFAdd %float %389 %391
        %393 = OpFMul %float %float_0_5 %392
        %394 = OpCompositeInsert %v4float %393 %123 0
        %395 = OpLoad %float %390
        %396 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_0 %int_3
        %397 = OpLoad %float %396
        %398 = OpFAdd %float %395 %397
        %399 = OpFMul %float %float_0_5 %398
        %400 = OpCompositeInsert %v4float %399 %394 1
        %401 = OpLoad %float %396
        %402 = OpLoad %float %388
        %403 = OpFAdd %float %401 %402
        %404 = OpFMul %float %float_0_5 %403
        %405 = OpCompositeInsert %v4float %404 %400 2
        %406 = OpLoad %float %396
        %407 = OpLoad %float %388
        %408 = OpFAdd %float %406 %407
        %409 = OpLoad %float %390
        %410 = OpFAdd %float %408 %409
        %411 = OpFMul %float %float_0_333000004 %410
        %412 = OpCompositeInsert %v4float %411 %405 3
        %413 = OpVectorShuffle %v3float %359 %359 0 1 2
        %414 = OpVectorShuffle %v3float %365 %365 0 1 2
        %415 = OpVectorShuffle %v3float %371 %371 0 1 2
               OpBranch %416
        %416 = OpLabel
               OpLoopMerge %417 %418 None
               OpBranch %419
        %419 = OpLabel
        %420 = OpMatrixTimesVector %v4float %353 %68
        %421 = OpCompositeExtract %float %359 0
        %422 = OpCompositeExtract %float %359 1
        %423 = OpCompositeExtract %float %359 2
        %424 = OpCompositeConstruct %v4float %421 %422 %423 %float_1
        %425 = OpMatrixTimesVector %v4float %351 %424
        %426 = OpVectorShuffle %v3float %425 %425 0 1 2
        %427 = OpVectorShuffle %v3float %420 %420 0 1 2
        %428 = OpFSub %v3float %426 %427
        %429 = OpCompositeExtract %float %425 3
        %430 = OpCompositeExtract %float %420 3
        %431 = OpFAdd %float %429 %430
        %432 = OpCompositeConstruct %v3float %431 %431 %431
        %433 = OpFOrdLessThan %v3bool %428 %432
        %434 = OpSelect %v3int %433 %74 %65
        %435 = OpFAdd %v3float %426 %427
        %436 = OpFNegate %float %429
        %437 = OpFSub %float %436 %430
        %438 = OpCompositeConstruct %v3float %437 %437 %437
        %439 = OpFOrdGreaterThan %v3bool %435 %438
        %440 = OpSelect %v3int %439 %74 %65
        %441 = OpIMul %v3int %75 %440
        %442 = OpIAdd %v3int %434 %441
        %443 = OpCompositeExtract %float %365 0
        %444 = OpCompositeExtract %float %365 1
        %445 = OpCompositeExtract %float %365 2
        %446 = OpCompositeConstruct %v4float %443 %444 %445 %float_1
        %447 = OpMatrixTimesVector %v4float %351 %446
        %448 = OpVectorShuffle %v3float %447 %447 0 1 2
        %449 = OpFSub %v3float %448 %427
        %450 = OpCompositeExtract %float %447 3
        %451 = OpFAdd %float %450 %430
        %452 = OpCompositeConstruct %v3float %451 %451 %451
        %453 = OpFOrdLessThan %v3bool %449 %452
        %454 = OpSelect %v3int %453 %74 %65
        %455 = OpFAdd %v3float %448 %427
        %456 = OpFNegate %float %450
        %457 = OpFSub %float %456 %430
        %458 = OpCompositeConstruct %v3float %457 %457 %457
        %459 = OpFOrdGreaterThan %v3bool %455 %458
        %460 = OpSelect %v3int %459 %74 %65
        %461 = OpIMul %v3int %75 %460
        %462 = OpIAdd %v3int %454 %461
        %463 = OpBitwiseOr %v3int %442 %462
        %464 = OpCompositeExtract %float %371 0
        %465 = OpCompositeExtract %float %371 1
        %466 = OpCompositeExtract %float %371 2
        %467 = OpCompositeConstruct %v4float %464 %465 %466 %float_1
        %468 = OpMatrixTimesVector %v4float %351 %467
        %469 = OpVectorShuffle %v3float %468 %468 0 1 2
        %470 = OpFSub %v3float %469 %427
        %471 = OpCompositeExtract %float %468 3
        %472 = OpFAdd %float %471 %430
        %473 = OpCompositeConstruct %v3float %472 %472 %472
        %474 = OpFOrdLessThan %v3bool %470 %473
        %475 = OpSelect %v3int %474 %74 %65
        %476 = OpFAdd %v3float %469 %427
        %477 = OpFNegate %float %471
        %478 = OpFSub %float %477 %430
        %479 = OpCompositeConstruct %v3float %478 %478 %478
        %480 = OpFOrdGreaterThan %v3bool %476 %479
        %481 = OpSelect %v3int %480 %74 %65
        %482 = OpIMul %v3int %75 %481
        %483 = OpIAdd %v3int %475 %482
        %484 = OpBitwiseOr %v3int %463 %483
        %485 = OpINotEqual %v3bool %484 %66
        %486 = OpAny %bool %485
               OpSelectionMerge %487 None
               OpBranchConditional %486 %488 %487
        %488 = OpLabel
               OpBranch %417
        %487 = OpLabel
        %489 = OpFSub %v3float %413 %414
        %490 = OpFSub %v3float %414 %415
        %491 = OpFSub %v3float %415 %413
        %492 = OpFAdd %v3float %413 %414
        %493 = OpFMul %v3float %69 %492
        %494 = OpFSub %v3float %493 %355
        %495 = OpFAdd %v3float %414 %415
        %496 = OpFMul %v3float %69 %495
        %497 = OpFSub %v3float %496 %355
        %498 = OpFAdd %v3float %415 %413
        %499 = OpFMul %v3float %69 %498
        %500 = OpFSub %v3float %499 %355
        %501 = OpDot %float %490 %490
        %502 = OpDot %float %497 %497
        %503 = OpFDiv %float %501 %502
        %504 = OpExtInst %float %1 Sqrt %503
        %505 = OpDot %float %491 %491
        %506 = OpDot %float %500 %500
        %507 = OpFDiv %float %505 %506
        %508 = OpExtInst %float %1 Sqrt %507
        %509 = OpDot %float %489 %489
        %510 = OpDot %float %494 %494
        %511 = OpFDiv %float %509 %510
        %512 = OpExtInst %float %1 Sqrt %511
        %513 = OpCompositeConstruct %v4float %504 %508 %512 %float_1
        %514 = OpFAdd %float %504 %508
        %515 = OpFAdd %float %514 %512
        %516 = OpFMul %float %float_0_333000004 %515
        %517 = OpCompositeInsert %v4float %516 %513 3
        %518 = OpCompositeConstruct %v4float %357 %357 %357 %357
        %519 = OpFMul %v4float %518 %517
               OpBranch %417
        %418 = OpLabel
               OpBranch %416
        %417 = OpLabel
        %520 = OpPhi %v4float %68 %488 %519 %487
        %521 = OpFMul %v4float %412 %520
        %522 = OpExtInst %v4float %1 FClamp %521 %59 %61
        %523 = OpCompositeExtract %float %522 0
        %524 = OpCompositeExtract %float %522 1
        %525 = OpCompositeExtract %float %522 2
        %526 = OpCompositeExtract %float %522 3
        %527 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_0
               OpStore %527 %523
        %528 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_1
               OpStore %528 %524
        %529 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_2
               OpStore %529 %525
        %530 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0
               OpStore %530 %526
               OpStore %out_var_PN_POSITION9 %387
               OpBranch %if_merge
   %if_merge = OpLabel
               OpReturn
               OpFunctionEnd
