; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 452
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainOnePassPointLightPS "main" %gl_FragCoord %out_var_SV_Target0
               OpExecutionMode %MainOnePassPointLightPS OriginUpperLeft
               OpSource HLSL 600
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
               OpName %type_2d_image "type.2d.image"
               OpName %SceneTexturesStruct_SceneDepthTexture "SceneTexturesStruct_SceneDepthTexture"
               OpName %type_sampler "type.sampler"
               OpName %SceneTexturesStruct_SceneDepthTextureSampler "SceneTexturesStruct_SceneDepthTextureSampler"
               OpName %SceneTexturesStruct_GBufferATexture "SceneTexturesStruct_GBufferATexture"
               OpName %SceneTexturesStruct_GBufferBTexture "SceneTexturesStruct_GBufferBTexture"
               OpName %SceneTexturesStruct_GBufferDTexture "SceneTexturesStruct_GBufferDTexture"
               OpName %SceneTexturesStruct_GBufferATextureSampler "SceneTexturesStruct_GBufferATextureSampler"
               OpName %SceneTexturesStruct_GBufferBTextureSampler "SceneTexturesStruct_GBufferBTextureSampler"
               OpName %SceneTexturesStruct_GBufferDTextureSampler "SceneTexturesStruct_GBufferDTextureSampler"
               OpName %ShadowDepthTextureSampler "ShadowDepthTextureSampler"
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "SoftTransitionScale"
               OpMemberName %type__Globals 1 "ShadowViewProjectionMatrices"
               OpMemberName %type__Globals 2 "InvShadowmapResolution"
               OpMemberName %type__Globals 3 "ShadowFadeFraction"
               OpMemberName %type__Globals 4 "ShadowSharpen"
               OpMemberName %type__Globals 5 "LightPositionAndInvRadius"
               OpMemberName %type__Globals 6 "ProjectionDepthBiasParameters"
               OpMemberName %type__Globals 7 "PointLightDepthBiasAndProjParameters"
               OpName %_Globals "$Globals"
               OpName %type_cube_image "type.cube.image"
               OpName %ShadowDepthCubeTexture "ShadowDepthCubeTexture"
               OpName %ShadowDepthCubeTextureSampler "ShadowDepthCubeTextureSampler"
               OpName %SSProfilesTexture "SSProfilesTexture"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainOnePassPointLightPS "MainOnePassPointLightPS"
               OpName %type_sampled_image "type.sampled.image"
               OpName %type_sampled_image_0 "type.sampled.image"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_POSITION"
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 0
               OpDecorate %SceneTexturesStruct_SceneDepthTexture DescriptorSet 0
               OpDecorate %SceneTexturesStruct_SceneDepthTexture Binding 0
               OpDecorate %SceneTexturesStruct_SceneDepthTextureSampler DescriptorSet 0
               OpDecorate %SceneTexturesStruct_SceneDepthTextureSampler Binding 0
               OpDecorate %SceneTexturesStruct_GBufferATexture DescriptorSet 0
               OpDecorate %SceneTexturesStruct_GBufferATexture Binding 1
               OpDecorate %SceneTexturesStruct_GBufferBTexture DescriptorSet 0
               OpDecorate %SceneTexturesStruct_GBufferBTexture Binding 2
               OpDecorate %SceneTexturesStruct_GBufferDTexture DescriptorSet 0
               OpDecorate %SceneTexturesStruct_GBufferDTexture Binding 3
               OpDecorate %SceneTexturesStruct_GBufferATextureSampler DescriptorSet 0
               OpDecorate %SceneTexturesStruct_GBufferATextureSampler Binding 1
               OpDecorate %SceneTexturesStruct_GBufferBTextureSampler DescriptorSet 0
               OpDecorate %SceneTexturesStruct_GBufferBTextureSampler Binding 2
               OpDecorate %SceneTexturesStruct_GBufferDTextureSampler DescriptorSet 0
               OpDecorate %SceneTexturesStruct_GBufferDTextureSampler Binding 3
               OpDecorate %ShadowDepthTextureSampler DescriptorSet 0
               OpDecorate %ShadowDepthTextureSampler Binding 4
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 1
               OpDecorate %ShadowDepthCubeTexture DescriptorSet 0
               OpDecorate %ShadowDepthCubeTexture Binding 4
               OpDecorate %ShadowDepthCubeTextureSampler DescriptorSet 0
               OpDecorate %ShadowDepthCubeTextureSampler Binding 5
               OpDecorate %SSProfilesTexture DescriptorSet 0
               OpDecorate %SSProfilesTexture Binding 5
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
               OpDecorate %_arr_mat4v4float_uint_6 ArrayStride 64
               OpMemberDecorate %type__Globals 0 Offset 0
               OpMemberDecorate %type__Globals 1 Offset 16
               OpMemberDecorate %type__Globals 1 MatrixStride 16
               OpMemberDecorate %type__Globals 1 ColMajor
               OpMemberDecorate %type__Globals 2 Offset 400
               OpMemberDecorate %type__Globals 3 Offset 404
               OpMemberDecorate %type__Globals 4 Offset 408
               OpMemberDecorate %type__Globals 5 Offset 416
               OpMemberDecorate %type__Globals 6 Offset 432
               OpMemberDecorate %type__Globals 7 Offset 448
               OpDecorate %type__Globals Block
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
    %float_0 = OpConstant %float 0
  %float_2_5 = OpConstant %float 2.5
%float_2_37764096 = OpConstant %float 2.37764096
%float_0_772542 = OpConstant %float 0.772542
%float_1_46946299 = OpConstant %float 1.46946299
%float_n2_02254295 = OpConstant %float -2.02254295
%float_n1_46946299 = OpConstant %float -1.46946299
%float_n2_022542 = OpConstant %float -2.022542
%float_n2_37764096 = OpConstant %float -2.37764096
%float_0_772543013 = OpConstant %float 0.772543013
    %float_1 = OpConstant %float 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_7 = OpConstant %int 7
     %int_58 = OpConstant %int 58
     %int_24 = OpConstant %int 24
     %int_11 = OpConstant %int 11
      %int_5 = OpConstant %int 5
  %float_0_5 = OpConstant %float 0.5
      %int_4 = OpConstant %int 4
      %int_2 = OpConstant %int 2
         %62 = OpConstantComposite %v3float %float_1 %float_1 %float_1
       %bool = OpTypeBool
     %uint_5 = OpConstant %uint 5
         %65 = OpConstantComposite %v3float %float_0 %float_0 %float_1
         %66 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
   %float_10 = OpConstant %float 10
    %float_5 = OpConstant %float 5
     %uint_0 = OpConstant %uint 0
     %int_23 = OpConstant %int 23
     %uint_1 = OpConstant %uint 1
     %uint_3 = OpConstant %uint 3
    %uint_16 = OpConstant %uint 16
%float_0_150000006 = OpConstant %float 0.150000006
 %float_0_25 = OpConstant %float 0.25
    %float_2 = OpConstant %float 2
         %77 = OpConstantComposite %v3float %float_2 %float_2 %float_2
  %float_255 = OpConstant %float 255
    %uint_15 = OpConstant %uint 15
%uint_4294967280 = OpConstant %uint 4294967280
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
     %uint_6 = OpConstant %uint 6
%_arr_mat4v4float_uint_6 = OpTypeArray %mat4v4float %uint_6
%type__Globals = OpTypeStruct %v3float %_arr_mat4v4float_uint_6 %float %float %float %v4float %v2float %v4float
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%type_cube_image = OpTypeImage %float Cube 2 0 0 1 Unknown
%_ptr_UniformConstant_type_cube_image = OpTypePointer UniformConstant %type_cube_image
      %v2int = OpTypeVector %int 2
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %91 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%type_sampled_image = OpTypeSampledImage %type_cube_image
      %v3int = OpTypeVector %int 3
%type_sampled_image_0 = OpTypeSampledImage %type_2d_image
     %v4bool = OpTypeVector %bool 4
       %View = OpVariable %_ptr_Uniform_type_View Uniform
%SceneTexturesStruct_SceneDepthTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%SceneTexturesStruct_SceneDepthTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%SceneTexturesStruct_GBufferATexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%SceneTexturesStruct_GBufferBTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%SceneTexturesStruct_GBufferDTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%SceneTexturesStruct_GBufferATextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%SceneTexturesStruct_GBufferBTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%SceneTexturesStruct_GBufferDTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%ShadowDepthTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%ShadowDepthCubeTexture = OpVariable %_ptr_UniformConstant_type_cube_image UniformConstant
%ShadowDepthCubeTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%SSProfilesTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
%float_0_200000003 = OpConstant %float 0.200000003
         %98 = OpConstantComposite %v3float %float_2_5 %float_2_5 %float_2_5
         %99 = OpConstantComposite %v3float %float_2_37764096 %float_2_37764096 %float_2_37764096
        %100 = OpConstantComposite %v3float %float_0_772542 %float_0_772542 %float_0_772542
        %101 = OpConstantComposite %v3float %float_1_46946299 %float_1_46946299 %float_1_46946299
        %102 = OpConstantComposite %v3float %float_n2_02254295 %float_n2_02254295 %float_n2_02254295
        %103 = OpConstantComposite %v3float %float_n1_46946299 %float_n1_46946299 %float_n1_46946299
        %104 = OpConstantComposite %v3float %float_n2_022542 %float_n2_022542 %float_n2_022542
        %105 = OpConstantComposite %v3float %float_n2_37764096 %float_n2_37764096 %float_n2_37764096
        %106 = OpConstantComposite %v3float %float_0_772543013 %float_0_772543013 %float_0_772543013
        %107 = OpUndef %v4float
%MainOnePassPointLightPS = OpFunction %void None %91
        %108 = OpLabel
        %109 = OpLoad %v4float %gl_FragCoord
        %110 = OpVectorShuffle %v2float %109 %109 0 1
        %111 = OpAccessChain %_ptr_Uniform_v4float %View %int_58
        %112 = OpLoad %v4float %111
        %113 = OpVectorShuffle %v2float %112 %112 2 3
        %114 = OpFMul %v2float %110 %113
        %115 = OpLoad %type_2d_image %SceneTexturesStruct_SceneDepthTexture
        %116 = OpLoad %type_sampler %SceneTexturesStruct_SceneDepthTextureSampler
        %117 = OpSampledImage %type_sampled_image_0 %115 %116
        %118 = OpImageSampleExplicitLod %v4float %117 %114 Lod %float_0
        %119 = OpCompositeExtract %float %118 0
        %120 = OpAccessChain %_ptr_Uniform_float %View %int_23 %uint_0
        %121 = OpLoad %float %120
        %122 = OpFMul %float %119 %121
        %123 = OpAccessChain %_ptr_Uniform_float %View %int_23 %uint_1
        %124 = OpLoad %float %123
        %125 = OpFAdd %float %122 %124
        %126 = OpAccessChain %_ptr_Uniform_float %View %int_23 %uint_2
        %127 = OpLoad %float %126
        %128 = OpFMul %float %119 %127
        %129 = OpAccessChain %_ptr_Uniform_float %View %int_23 %uint_3
        %130 = OpLoad %float %129
        %131 = OpFSub %float %128 %130
        %132 = OpFDiv %float %float_1 %131
        %133 = OpFAdd %float %125 %132
        %134 = OpAccessChain %_ptr_Uniform_v4float %View %int_24
        %135 = OpLoad %v4float %134
        %136 = OpVectorShuffle %v2float %135 %135 3 2
        %137 = OpFSub %v2float %114 %136
        %138 = OpVectorShuffle %v2float %135 %135 0 1
        %139 = OpFDiv %v2float %137 %138
        %140 = OpCompositeConstruct %v2float %133 %133
        %141 = OpFMul %v2float %139 %140
        %142 = OpCompositeExtract %float %141 0
        %143 = OpCompositeExtract %float %141 1
        %144 = OpCompositeConstruct %v4float %142 %143 %133 %float_1
        %145 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_11
        %146 = OpLoad %mat4v4float %145
        %147 = OpMatrixTimesVector %v4float %146 %144
        %148 = OpVectorShuffle %v3float %147 %147 0 1 2
        %149 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_5
        %150 = OpLoad %v4float %149
        %151 = OpVectorShuffle %v3float %150 %150 0 1 2
        %152 = OpFSub %v3float %151 %148
        %153 = OpAccessChain %_ptr_Uniform_float %_Globals %int_5 %int_3
        %154 = OpLoad %float %153
        %155 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_7
        %156 = OpAccessChain %_ptr_Uniform_float %_Globals %int_7 %int_0
        %157 = OpLoad %float %156
        %158 = OpExtInst %float %1 Length %152
        %159 = OpFMul %float %158 %154
        %160 = OpFOrdLessThan %bool %159 %float_1
               OpSelectionMerge %161 DontFlatten
               OpBranchConditional %160 %162 %161
        %162 = OpLabel
        %163 = OpCompositeConstruct %v3float %158 %158 %158
        %164 = OpFDiv %v3float %152 %163
        %165 = OpExtInst %v3float %1 FAbs %152
        %166 = OpCompositeExtract %float %165 0
        %167 = OpCompositeExtract %float %165 1
        %168 = OpCompositeExtract %float %165 2
        %169 = OpExtInst %float %1 FMax %167 %168
        %170 = OpExtInst %float %1 FMax %166 %169
        %171 = OpFOrdEqual %bool %170 %166
               OpSelectionMerge %172 None
               OpBranchConditional %171 %173 %174
        %174 = OpLabel
        %175 = OpFOrdEqual %bool %170 %167
               OpSelectionMerge %176 None
               OpBranchConditional %175 %177 %178
        %178 = OpLabel
        %179 = OpCompositeExtract %float %152 2
        %180 = OpFOrdEqual %bool %168 %179
        %181 = OpSelect %int %180 %int_4 %int_5
               OpBranch %176
        %177 = OpLabel
        %182 = OpCompositeExtract %float %152 1
        %183 = OpFOrdEqual %bool %167 %182
        %184 = OpSelect %int %183 %int_2 %int_3
               OpBranch %176
        %176 = OpLabel
        %185 = OpPhi %int %184 %177 %181 %178
               OpBranch %172
        %173 = OpLabel
        %186 = OpCompositeExtract %float %152 0
        %187 = OpFOrdEqual %bool %166 %186
        %188 = OpSelect %int %187 %int_0 %int_1
               OpBranch %172
        %172 = OpLabel
        %189 = OpPhi %int %188 %173 %185 %176
        %190 = OpCompositeExtract %float %147 0
        %191 = OpCompositeExtract %float %147 1
        %192 = OpCompositeExtract %float %147 2
        %193 = OpCompositeConstruct %v4float %190 %191 %192 %float_1
        %194 = OpAccessChain %_ptr_Uniform_mat4v4float %_Globals %int_1 %189
        %195 = OpLoad %mat4v4float %194
        %196 = OpMatrixTimesVector %v4float %195 %193
        %197 = OpCompositeExtract %float %196 2
        %198 = OpCompositeExtract %float %196 3
        %199 = OpFDiv %float %197 %198
        %200 = OpFNegate %float %157
        %201 = OpFDiv %float %200 %198
        %202 = OpLoad %type_cube_image %ShadowDepthCubeTexture
        %203 = OpLoad %type_sampler %ShadowDepthCubeTextureSampler
        %204 = OpFAdd %float %199 %201
        %205 = OpSampledImage %type_sampled_image %202 %203
        %206 = OpImageSampleDrefExplicitLod %float %205 %164 %204 Lod %float_0
               OpBranch %161
        %161 = OpLabel
        %207 = OpPhi %float %float_1 %108 %206 %172
        %208 = OpFSub %float %207 %float_0_5
        %209 = OpAccessChain %_ptr_Uniform_float %_Globals %int_4
        %210 = OpLoad %float %209
        %211 = OpFMul %float %208 %210
        %212 = OpFAdd %float %211 %float_0_5
        %213 = OpExtInst %float %1 FClamp %212 %float_0 %float_1
        %214 = OpFMul %float %213 %213
        %215 = OpAccessChain %_ptr_Uniform_float %_Globals %int_3
        %216 = OpLoad %float %215
        %217 = OpExtInst %float %1 FMix %float_1 %214 %216
        %218 = OpExtInst %float %1 Sqrt %217
        %219 = OpCompositeInsert %v4float %218 %107 2
        %220 = OpVectorShuffle %v4float %219 %62 4 5 2 6
        %221 = OpLoad %type_2d_image %SceneTexturesStruct_GBufferATexture
        %222 = OpLoad %type_sampler %SceneTexturesStruct_GBufferATextureSampler
        %223 = OpSampledImage %type_sampled_image_0 %221 %222
        %224 = OpImageSampleExplicitLod %v4float %223 %114 Lod %float_0
        %225 = OpLoad %type_2d_image %SceneTexturesStruct_GBufferBTexture
        %226 = OpLoad %type_sampler %SceneTexturesStruct_GBufferBTextureSampler
        %227 = OpSampledImage %type_sampled_image_0 %225 %226
        %228 = OpImageSampleExplicitLod %v4float %227 %114 Lod %float_0
        %229 = OpLoad %type_2d_image %SceneTexturesStruct_GBufferDTexture
        %230 = OpLoad %type_sampler %SceneTexturesStruct_GBufferDTextureSampler
        %231 = OpSampledImage %type_sampled_image_0 %229 %230
        %232 = OpImageSampleExplicitLod %v4float %231 %114 Lod %float_0
        %233 = OpVectorShuffle %v3float %224 %224 0 1 2
        %234 = OpFMul %v3float %233 %77
        %235 = OpFSub %v3float %234 %62
        %236 = OpExtInst %v3float %1 Normalize %235
        %237 = OpCompositeExtract %float %228 3
        %238 = OpFMul %float %237 %float_255
        %239 = OpExtInst %float %1 Round %238
        %240 = OpConvertFToU %uint %239
        %241 = OpBitwiseAnd %uint %240 %uint_15
        %242 = OpBitwiseAnd %uint %240 %uint_4294967280
        %243 = OpBitwiseAnd %uint %242 %uint_16
        %244 = OpINotEqual %bool %243 %uint_0
        %245 = OpLogicalNot %bool %244
        %246 = OpCompositeConstruct %v4bool %245 %245 %245 %245
        %247 = OpSelect %v4float %246 %232 %66
        %248 = OpIEqual %bool %241 %uint_5
               OpSelectionMerge %249 None
               OpBranchConditional %248 %250 %249
        %250 = OpLabel
        %251 = OpLoad %v4float %155
        %252 = OpCompositeExtract %float %247 0
        %253 = OpFMul %float %252 %float_255
        %254 = OpFAdd %float %253 %float_0_5
        %255 = OpConvertFToU %uint %254
        %256 = OpBitcast %int %255
        %257 = OpCompositeConstruct %v3int %int_1 %256 %int_0
        %258 = OpVectorShuffle %v2int %257 %257 0 1
        %259 = OpLoad %type_2d_image %SSProfilesTexture
        %260 = OpImageFetch %v4float %259 %258 Lod %int_0
        %261 = OpCompositeExtract %float %260 0
        %262 = OpCompositeExtract %float %260 1
        %263 = OpFMul %float %262 %float_0_5
        %264 = OpCompositeConstruct %v3float %263 %263 %263
        %265 = OpFMul %v3float %236 %264
        %266 = OpFSub %v3float %148 %265
        %267 = OpDot %float %152 %152
        %268 = OpExtInst %float %1 InverseSqrt %267
        %269 = OpCompositeConstruct %v3float %268 %268 %268
        %270 = OpFMul %v3float %152 %269
        %271 = OpFNegate %v3float %270
        %272 = OpDot %float %271 %236
        %273 = OpExtInst %float %1 FClamp %272 %float_0 %float_1
        %274 = OpExtInst %float %1 Pow %273 %float_1
               OpSelectionMerge %275 DontFlatten
               OpBranchConditional %160 %276 %275
        %276 = OpLabel
        %277 = OpCompositeConstruct %v3float %158 %158 %158
        %278 = OpFDiv %v3float %152 %277
        %279 = OpExtInst %v3float %1 Cross %278 %65
        %280 = OpExtInst %v3float %1 Normalize %279
        %281 = OpExtInst %v3float %1 Cross %280 %278
        %282 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2
        %283 = OpLoad %float %282
        %284 = OpCompositeConstruct %v3float %283 %283 %283
        %285 = OpFMul %v3float %280 %284
        %286 = OpFMul %v3float %281 %284
        %287 = OpExtInst %v3float %1 FAbs %278
        %288 = OpCompositeExtract %float %287 0
        %289 = OpCompositeExtract %float %287 1
        %290 = OpCompositeExtract %float %287 2
        %291 = OpExtInst %float %1 FMax %289 %290
        %292 = OpExtInst %float %1 FMax %288 %291
        %293 = OpFOrdEqual %bool %292 %288
               OpSelectionMerge %294 None
               OpBranchConditional %293 %295 %296
        %296 = OpLabel
        %297 = OpFOrdEqual %bool %292 %289
               OpSelectionMerge %298 None
               OpBranchConditional %297 %299 %300
        %300 = OpLabel
        %301 = OpCompositeExtract %float %278 2
        %302 = OpFOrdEqual %bool %290 %301
        %303 = OpSelect %int %302 %int_4 %int_5
               OpBranch %298
        %299 = OpLabel
        %304 = OpCompositeExtract %float %278 1
        %305 = OpFOrdEqual %bool %289 %304
        %306 = OpSelect %int %305 %int_2 %int_3
               OpBranch %298
        %298 = OpLabel
        %307 = OpPhi %int %306 %299 %303 %300
               OpBranch %294
        %295 = OpLabel
        %308 = OpCompositeExtract %float %278 0
        %309 = OpFOrdEqual %bool %288 %308
        %310 = OpSelect %int %309 %int_0 %int_1
               OpBranch %294
        %294 = OpLabel
        %311 = OpPhi %int %310 %295 %307 %298
        %312 = OpCompositeExtract %float %266 0
        %313 = OpCompositeExtract %float %266 1
        %314 = OpCompositeExtract %float %266 2
        %315 = OpCompositeConstruct %v4float %312 %313 %314 %float_1
        %316 = OpAccessChain %_ptr_Uniform_mat4v4float %_Globals %int_1 %311
        %317 = OpLoad %mat4v4float %316
        %318 = OpMatrixTimesVector %v4float %317 %315
        %319 = OpCompositeExtract %float %318 2
        %320 = OpCompositeExtract %float %318 3
        %321 = OpFDiv %float %319 %320
        %322 = OpFDiv %float %float_10 %154
        %323 = OpFMul %float %261 %322
        %324 = OpCompositeExtract %float %251 2
        %325 = OpFMul %float %321 %324
        %326 = OpCompositeExtract %float %251 3
        %327 = OpFSub %float %325 %326
        %328 = OpFDiv %float %float_1 %327
        %329 = OpFMul %float %328 %154
        %330 = OpFMul %v3float %286 %98
        %331 = OpFAdd %v3float %278 %330
        %332 = OpLoad %type_cube_image %ShadowDepthCubeTexture
        %333 = OpLoad %type_sampler %ShadowDepthTextureSampler
        %334 = OpSampledImage %type_sampled_image %332 %333
        %335 = OpImageSampleExplicitLod %v4float %334 %331 Lod %float_0
        %336 = OpCompositeExtract %float %335 0
        %337 = OpFMul %float %336 %324
        %338 = OpFSub %float %337 %326
        %339 = OpFDiv %float %float_1 %338
        %340 = OpFMul %float %339 %154
        %341 = OpFSub %float %329 %340
        %342 = OpFMul %float %341 %323
        %343 = OpFOrdGreaterThan %bool %342 %float_0
        %344 = OpFAdd %float %342 %263
        %345 = OpFMul %float %342 %274
        %346 = OpFAdd %float %345 %263
        %347 = OpExtInst %float %1 FMax %float_0 %346
        %348 = OpSelect %float %343 %344 %347
        %349 = OpExtInst %float %1 FAbs %348
        %350 = OpExtInst %float %1 FClamp %349 %float_0_150000006 %float_5
        %351 = OpFAdd %float %350 %float_0_25
        %352 = OpFMul %v3float %285 %99
        %353 = OpFAdd %v3float %278 %352
        %354 = OpFMul %v3float %286 %100
        %355 = OpFAdd %v3float %353 %354
        %356 = OpSampledImage %type_sampled_image %332 %333
        %357 = OpImageSampleExplicitLod %v4float %356 %355 Lod %float_0
        %358 = OpCompositeExtract %float %357 0
        %359 = OpFMul %float %358 %324
        %360 = OpFSub %float %359 %326
        %361 = OpFDiv %float %float_1 %360
        %362 = OpFMul %float %361 %154
        %363 = OpFSub %float %329 %362
        %364 = OpFMul %float %363 %323
        %365 = OpFOrdGreaterThan %bool %364 %float_0
        %366 = OpFAdd %float %364 %263
        %367 = OpFMul %float %364 %274
        %368 = OpFAdd %float %367 %263
        %369 = OpExtInst %float %1 FMax %float_0 %368
        %370 = OpSelect %float %365 %366 %369
        %371 = OpExtInst %float %1 FAbs %370
        %372 = OpExtInst %float %1 FClamp %371 %float_0_150000006 %float_5
        %373 = OpFAdd %float %372 %float_0_25
        %374 = OpFAdd %float %351 %373
        %375 = OpFMul %v3float %285 %101
        %376 = OpFAdd %v3float %278 %375
        %377 = OpFMul %v3float %286 %102
        %378 = OpFAdd %v3float %376 %377
        %379 = OpSampledImage %type_sampled_image %332 %333
        %380 = OpImageSampleExplicitLod %v4float %379 %378 Lod %float_0
        %381 = OpCompositeExtract %float %380 0
        %382 = OpFMul %float %381 %324
        %383 = OpFSub %float %382 %326
        %384 = OpFDiv %float %float_1 %383
        %385 = OpFMul %float %384 %154
        %386 = OpFSub %float %329 %385
        %387 = OpFMul %float %386 %323
        %388 = OpFOrdGreaterThan %bool %387 %float_0
        %389 = OpFAdd %float %387 %263
        %390 = OpFMul %float %387 %274
        %391 = OpFAdd %float %390 %263
        %392 = OpExtInst %float %1 FMax %float_0 %391
        %393 = OpSelect %float %388 %389 %392
        %394 = OpExtInst %float %1 FAbs %393
        %395 = OpExtInst %float %1 FClamp %394 %float_0_150000006 %float_5
        %396 = OpFAdd %float %395 %float_0_25
        %397 = OpFAdd %float %374 %396
        %398 = OpFMul %v3float %285 %103
        %399 = OpFAdd %v3float %278 %398
        %400 = OpFMul %v3float %286 %104
        %401 = OpFAdd %v3float %399 %400
        %402 = OpSampledImage %type_sampled_image %332 %333
        %403 = OpImageSampleExplicitLod %v4float %402 %401 Lod %float_0
        %404 = OpCompositeExtract %float %403 0
        %405 = OpFMul %float %404 %324
        %406 = OpFSub %float %405 %326
        %407 = OpFDiv %float %float_1 %406
        %408 = OpFMul %float %407 %154
        %409 = OpFSub %float %329 %408
        %410 = OpFMul %float %409 %323
        %411 = OpFOrdGreaterThan %bool %410 %float_0
        %412 = OpFAdd %float %410 %263
        %413 = OpFMul %float %410 %274
        %414 = OpFAdd %float %413 %263
        %415 = OpExtInst %float %1 FMax %float_0 %414
        %416 = OpSelect %float %411 %412 %415
        %417 = OpExtInst %float %1 FAbs %416
        %418 = OpExtInst %float %1 FClamp %417 %float_0_150000006 %float_5
        %419 = OpFAdd %float %418 %float_0_25
        %420 = OpFAdd %float %397 %419
        %421 = OpFMul %v3float %285 %105
        %422 = OpFAdd %v3float %278 %421
        %423 = OpFMul %v3float %286 %106
        %424 = OpFAdd %v3float %422 %423
        %425 = OpSampledImage %type_sampled_image %332 %333
        %426 = OpImageSampleExplicitLod %v4float %425 %424 Lod %float_0
        %427 = OpCompositeExtract %float %426 0
        %428 = OpFMul %float %427 %324
        %429 = OpFSub %float %428 %326
        %430 = OpFDiv %float %float_1 %429
        %431 = OpFMul %float %430 %154
        %432 = OpFSub %float %329 %431
        %433 = OpFMul %float %432 %323
        %434 = OpFOrdGreaterThan %bool %433 %float_0
        %435 = OpFAdd %float %433 %263
        %436 = OpFMul %float %433 %274
        %437 = OpFAdd %float %436 %263
        %438 = OpExtInst %float %1 FMax %float_0 %437
        %439 = OpSelect %float %434 %435 %438
        %440 = OpExtInst %float %1 FAbs %439
        %441 = OpExtInst %float %1 FClamp %440 %float_0_150000006 %float_5
        %442 = OpFAdd %float %441 %float_0_25
        %443 = OpFAdd %float %420 %442
        %444 = OpFMul %float %443 %float_0_200000003
               OpBranch %275
        %275 = OpLabel
        %445 = OpPhi %float %float_1 %250 %444 %294
        %446 = OpFMul %float %445 %float_0_200000003
        %447 = OpFSub %float %float_1 %446
               OpBranch %249
        %249 = OpLabel
        %448 = OpPhi %float %float_1 %161 %447 %275
        %449 = OpExtInst %float %1 Sqrt %448
        %450 = OpSelect %float %248 %449 %218
        %451 = OpCompositeInsert %v4float %450 %220 3
               OpStore %out_var_SV_Target0 %451
               OpReturn
               OpFunctionEnd
