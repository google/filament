; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 572
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %Main "main" %in_var_TEXCOORD0 %in_var_TEXCOORD7 %in_var_TEXCOORD8 %gl_FragCoord %gl_FrontFacing %out_var_SV_Target0
               OpExecutionMode %Main OriginUpperLeft
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
               OpName %type_MobileDirectionalLight "type.MobileDirectionalLight"
               OpMemberName %type_MobileDirectionalLight 0 "MobileDirectionalLight_DirectionalLightColor"
               OpMemberName %type_MobileDirectionalLight 1 "MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition"
               OpMemberName %type_MobileDirectionalLight 2 "MobileDirectionalLight_DirectionalLightShadowSize"
               OpMemberName %type_MobileDirectionalLight 3 "MobileDirectionalLight_DirectionalLightDistanceFadeMAD"
               OpMemberName %type_MobileDirectionalLight 4 "MobileDirectionalLight_DirectionalLightShadowDistances"
               OpMemberName %type_MobileDirectionalLight 5 "MobileDirectionalLight_DirectionalLightScreenToShadow"
               OpName %MobileDirectionalLight "MobileDirectionalLight"
               OpName %type_2d_image "type.2d.image"
               OpName %MobileDirectionalLight_DirectionalLightShadowTexture "MobileDirectionalLight_DirectionalLightShadowTexture"
               OpName %type_sampler "type.sampler"
               OpName %MobileDirectionalLight_DirectionalLightShadowSampler "MobileDirectionalLight_DirectionalLightShadowSampler"
               OpName %Material_Texture2D_0 "Material_Texture2D_0"
               OpName %Material_Texture2D_0Sampler "Material_Texture2D_0Sampler"
               OpName %Material_Texture2D_1 "Material_Texture2D_1"
               OpName %Material_Texture2D_1Sampler "Material_Texture2D_1Sampler"
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "NumDynamicPointLights"
               OpMemberName %type__Globals 1 "LightPositionAndInvRadius"
               OpMemberName %type__Globals 2 "LightColorAndFalloffExponent"
               OpMemberName %type__Globals 3 "MobileReflectionParams"
               OpName %_Globals "$Globals"
               OpName %type_cube_image "type.cube.image"
               OpName %ReflectionCubemap "ReflectionCubemap"
               OpName %ReflectionCubemapSampler "ReflectionCubemapSampler"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %in_var_TEXCOORD7 "in.var.TEXCOORD7"
               OpName %in_var_TEXCOORD8 "in.var.TEXCOORD8"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %Main "Main"
               OpName %type_sampled_image "type.sampled.image"
               OpName %type_sampled_image_0 "type.sampled.image"
               OpDecorateString %in_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %in_var_TEXCOORD7 UserSemantic "TEXCOORD7"
               OpDecorateString %in_var_TEXCOORD8 UserSemantic "TEXCOORD8"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_Position"
               OpDecorate %gl_FrontFacing BuiltIn FrontFacing
               OpDecorateString %gl_FrontFacing UserSemantic "SV_IsFrontFace"
               OpDecorate %gl_FrontFacing Flat
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %in_var_TEXCOORD0 Location 0
               OpDecorate %in_var_TEXCOORD7 Location 1
               OpDecorate %in_var_TEXCOORD8 Location 2
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 0
               OpDecorate %MobileDirectionalLight DescriptorSet 0
               OpDecorate %MobileDirectionalLight Binding 1
               OpDecorate %MobileDirectionalLight_DirectionalLightShadowTexture DescriptorSet 0
               OpDecorate %MobileDirectionalLight_DirectionalLightShadowTexture Binding 0
               OpDecorate %MobileDirectionalLight_DirectionalLightShadowSampler DescriptorSet 0
               OpDecorate %MobileDirectionalLight_DirectionalLightShadowSampler Binding 0
               OpDecorate %Material_Texture2D_0 DescriptorSet 0
               OpDecorate %Material_Texture2D_0 Binding 1
               OpDecorate %Material_Texture2D_0Sampler DescriptorSet 0
               OpDecorate %Material_Texture2D_0Sampler Binding 1
               OpDecorate %Material_Texture2D_1 DescriptorSet 0
               OpDecorate %Material_Texture2D_1 Binding 2
               OpDecorate %Material_Texture2D_1Sampler DescriptorSet 0
               OpDecorate %Material_Texture2D_1Sampler Binding 2
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 2
               OpDecorate %ReflectionCubemap DescriptorSet 0
               OpDecorate %ReflectionCubemap Binding 3
               OpDecorate %ReflectionCubemapSampler DescriptorSet 0
               OpDecorate %ReflectionCubemapSampler Binding 3
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
               OpDecorate %_arr_mat4v4float_uint_4 ArrayStride 64
               OpMemberDecorate %type_MobileDirectionalLight 0 Offset 0
               OpMemberDecorate %type_MobileDirectionalLight 1 Offset 16
               OpMemberDecorate %type_MobileDirectionalLight 2 Offset 32
               OpMemberDecorate %type_MobileDirectionalLight 3 Offset 48
               OpMemberDecorate %type_MobileDirectionalLight 4 Offset 64
               OpMemberDecorate %type_MobileDirectionalLight 5 Offset 80
               OpMemberDecorate %type_MobileDirectionalLight 5 MatrixStride 16
               OpMemberDecorate %type_MobileDirectionalLight 5 ColMajor
               OpDecorate %type_MobileDirectionalLight Block
               OpMemberDecorate %type__Globals 0 Offset 0
               OpMemberDecorate %type__Globals 1 Offset 16
               OpMemberDecorate %type__Globals 2 Offset 80
               OpMemberDecorate %type__Globals 3 Offset 144
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
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
      %int_4 = OpConstant %int 4
    %float_0 = OpConstant %float 0
      %int_3 = OpConstant %int 3
         %47 = OpConstantComposite %v3float %float_0 %float_0 %float_0
    %float_1 = OpConstant %float 1
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
      %int_5 = OpConstant %int 5
         %52 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%float_0_999989986 = OpConstant %float 0.999989986
%float_65000 = OpConstant %float 65000
         %55 = OpConstantComposite %v3float %float_65000 %float_65000 %float_65000
%float_0_318309873 = OpConstant %float 0.318309873
         %57 = OpConstantComposite %v3float %float_0_318309873 %float_0_318309873 %float_0_318309873
%float_65500 = OpConstant %float 65500
  %float_0_5 = OpConstant %float 0.5
         %60 = OpConstantComposite %v2float %float_0_5 %float_0_5
    %float_2 = OpConstant %float 2
   %float_n2 = OpConstant %float -2
         %63 = OpConstantComposite %v2float %float_2 %float_n2
         %64 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%float_0_119999997 = OpConstant %float 0.119999997
   %float_n1 = OpConstant %float -1
%float_n0_0274999999 = OpConstant %float -0.0274999999
         %68 = OpConstantComposite %v2float %float_n1 %float_n0_0274999999
%float_0_0425000004 = OpConstant %float 0.0425000004
         %70 = OpConstantComposite %v2float %float_1 %float_0_0425000004
%float_n9_27999973 = OpConstant %float -9.27999973
         %72 = OpConstantComposite %v2float %float_1 %float_1
 %float_0_25 = OpConstant %float 0.25
   %float_16 = OpConstant %float 16
     %int_31 = OpConstant %int 31
     %int_56 = OpConstant %int 56
     %int_57 = OpConstant %int 57
     %int_64 = OpConstant %int 64
     %int_65 = OpConstant %int 65
     %int_66 = OpConstant %int 66
     %int_67 = OpConstant %int 67
     %int_88 = OpConstant %int 88
    %int_135 = OpConstant %int 135
    %int_139 = OpConstant %int 139
%mat3v3float = OpTypeMatrix %v3float 3
         %86 = OpConstantComposite %v2float %float_2 %float_2
%float_0_300000012 = OpConstant %float 0.300000012
         %88 = OpConstantComposite %v3float %float_0_300000012 %float_0_300000012 %float_1
   %float_20 = OpConstant %float 20
         %90 = OpConstantComposite %v2float %float_20 %float_20
%float_0_400000006 = OpConstant %float 0.400000006
   %float_24 = OpConstant %float 24
%float_0_294999987 = OpConstant %float 0.294999987
%float_0_660000026 = OpConstant %float 0.660000026
%float_0_699999988 = OpConstant %float 0.699999988
%float_65504 = OpConstant %float 65504
%float_1_20000005 = OpConstant %float 1.20000005
         %98 = OpConstantComposite %v3float %float_2 %float_2 %float_2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%_arr_mat4v4float_uint_4 = OpTypeArray %mat4v4float %uint_4
%type_MobileDirectionalLight = OpTypeStruct %v4float %v4float %v4float %v4float %v4float %_arr_mat4v4float_uint_4
%_ptr_Uniform_type_MobileDirectionalLight = OpTypePointer Uniform %type_MobileDirectionalLight
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%type__Globals = OpTypeStruct %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %v4float
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%type_cube_image = OpTypeImage %float Cube 2 0 0 1 Unknown
%_ptr_UniformConstant_type_cube_image = OpTypePointer UniformConstant %type_cube_image
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_bool = OpTypePointer Input %bool
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
        %110 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_int = OpTypePointer Uniform %int
%type_sampled_image = OpTypeSampledImage %type_cube_image
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%type_sampled_image_0 = OpTypeSampledImage %type_2d_image
       %View = OpVariable %_ptr_Uniform_type_View Uniform
%MobileDirectionalLight = OpVariable %_ptr_Uniform_type_MobileDirectionalLight Uniform
%MobileDirectionalLight_DirectionalLightShadowTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%MobileDirectionalLight_DirectionalLightShadowSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%Material_Texture2D_0 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%Material_Texture2D_0Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%Material_Texture2D_1 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%Material_Texture2D_1Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%ReflectionCubemap = OpVariable %_ptr_UniformConstant_type_cube_image UniformConstant
%ReflectionCubemapSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%in_var_TEXCOORD0 = OpVariable %_ptr_Input_v2float Input
%in_var_TEXCOORD7 = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD8 = OpVariable %_ptr_Input_v4float Input
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%gl_FrontFacing = OpVariable %_ptr_Input_bool Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
        %117 = OpConstantComposite %v3float %float_1 %float_0 %float_0
        %118 = OpConstantComposite %v3float %float_0 %float_1 %float_0
        %119 = OpConstantComposite %v3float %float_0 %float_0 %float_1
        %120 = OpConstantComposite %mat3v3float %117 %118 %119
   %float_10 = OpConstant %float 10
        %122 = OpConstantComposite %v2float %float_10 %float_10
    %float_5 = OpConstant %float 5
        %124 = OpConstantComposite %v2float %float_5 %float_5
%float_0_00066666666 = OpConstant %float 0.00066666666
 %float_n0_5 = OpConstant %float -0.5
        %127 = OpConstantComposite %v2float %float_n0_5 %float_n0_5
        %128 = OpConstantComposite %v2float %float_0_5 %float_n0_5
  %float_1_5 = OpConstant %float 1.5
        %130 = OpConstantComposite %v2float %float_1_5 %float_n0_5
        %131 = OpConstantComposite %v2float %float_n0_5 %float_0_5
        %132 = OpConstantComposite %v2float %float_1_5 %float_0_5
        %133 = OpConstantComposite %v2float %float_n0_5 %float_1_5
        %134 = OpConstantComposite %v2float %float_0_5 %float_1_5
        %135 = OpConstantComposite %v2float %float_1_5 %float_1_5
        %136 = OpUndef %v3float
        %137 = OpUndef %v4float
        %138 = OpUndef %float
        %139 = OpUndef %v3float
       %Main = OpFunction %void None %110
        %140 = OpLabel
        %141 = OpLoad %v2float %in_var_TEXCOORD0
        %142 = OpLoad %v4float %in_var_TEXCOORD7
        %143 = OpLoad %v4float %in_var_TEXCOORD8
        %144 = OpLoad %v4float %gl_FragCoord
        %145 = OpAccessChain %_ptr_Uniform_v3float %View %int_31
        %146 = OpLoad %v3float %145
        %147 = OpAccessChain %_ptr_Uniform_v4float %View %int_56
        %148 = OpLoad %v4float %147
        %149 = OpAccessChain %_ptr_Uniform_v4float %View %int_57
        %150 = OpLoad %v4float %149
        %151 = OpAccessChain %_ptr_Uniform_v4float %View %int_64
        %152 = OpLoad %v4float %151
        %153 = OpAccessChain %_ptr_Uniform_v4float %View %int_65
        %154 = OpLoad %v4float %153
        %155 = OpAccessChain %_ptr_Uniform_v4float %View %int_66
        %156 = OpLoad %v4float %155
        %157 = OpAccessChain %_ptr_Uniform_v2float %View %int_67
        %158 = OpLoad %v2float %157
        %159 = OpAccessChain %_ptr_Uniform_float %View %int_88
        %160 = OpLoad %float %159
        %161 = OpAccessChain %_ptr_Uniform_v4float %View %int_135
        %162 = OpLoad %v4float %161
        %163 = OpAccessChain %_ptr_Uniform_float %View %int_139
        %164 = OpLoad %float %163
        %165 = OpVectorShuffle %v2float %144 %144 0 1
        %166 = OpVectorShuffle %v2float %148 %148 0 1
        %167 = OpFSub %v2float %165 %166
        %168 = OpVectorShuffle %v2float %150 %150 2 3
        %169 = OpFMul %v2float %167 %168
        %170 = OpFSub %v2float %169 %60
        %171 = OpFMul %v2float %170 %63
        %172 = OpCompositeExtract %float %171 0
        %173 = OpCompositeExtract %float %171 1
        %174 = OpCompositeConstruct %v4float %172 %173 %138 %float_1
        %175 = OpCompositeExtract %float %144 3
        %176 = OpCompositeConstruct %v4float %175 %175 %175 %175
        %177 = OpFMul %v4float %174 %176
        %178 = OpVectorShuffle %v3float %143 %143 0 1 2
        %179 = OpFSub %v3float %178 %146
        %180 = OpFNegate %v3float %178
        %181 = OpExtInst %v3float %1 Normalize %180
        %182 = OpFMul %v2float %141 %60
        %183 = OpFMul %v2float %141 %122
        %184 = OpLoad %type_2d_image %Material_Texture2D_0
        %185 = OpLoad %type_sampler %Material_Texture2D_0Sampler
        %186 = OpSampledImage %type_sampled_image_0 %184 %185
        %187 = OpImageSampleImplicitLod %v4float %186 %183 None
        %188 = OpVectorShuffle %v2float %187 %187 0 1
        %189 = OpFMul %v2float %188 %86
        %190 = OpFSub %v2float %189 %72
        %191 = OpDot %float %190 %190
        %192 = OpFSub %float %float_1 %191
        %193 = OpExtInst %float %1 FClamp %192 %float_0 %float_1
        %194 = OpExtInst %float %1 Sqrt %193
        %195 = OpCompositeExtract %float %190 0
        %196 = OpCompositeExtract %float %190 1
        %197 = OpCompositeConstruct %v4float %195 %196 %194 %float_1
        %198 = OpVectorShuffle %v3float %197 %197 0 1 2
        %199 = OpFMul %v3float %198 %88
        %200 = OpVectorShuffle %v3float %156 %156 0 1 2
        %201 = OpCompositeExtract %float %156 3
        %202 = OpCompositeConstruct %v3float %201 %201 %201
        %203 = OpFMul %v3float %199 %202
        %204 = OpFAdd %v3float %203 %200
        %205 = OpMatrixTimesVector %v3float %120 %204
        %206 = OpExtInst %v3float %1 Normalize %205
        %207 = OpFNegate %v3float %181
        %208 = OpDot %float %206 %181
        %209 = OpCompositeConstruct %v3float %208 %208 %208
        %210 = OpFMul %v3float %206 %209
        %211 = OpFMul %v3float %210 %98
        %212 = OpFAdd %v3float %207 %211
        %213 = OpFMul %v2float %141 %90
        %214 = OpLoad %type_2d_image %Material_Texture2D_1
        %215 = OpLoad %type_sampler %Material_Texture2D_1Sampler
        %216 = OpSampledImage %type_sampled_image_0 %214 %215
        %217 = OpImageSampleImplicitLod %v4float %216 %213 None
        %218 = OpCompositeExtract %float %217 0
        %219 = OpExtInst %float %1 FMix %float_0_400000006 %float_1 %218
        %220 = OpFSub %float %float_1 %219
        %221 = OpFMul %v2float %141 %124
        %222 = OpSampledImage %type_sampled_image_0 %214 %215
        %223 = OpImageSampleImplicitLod %v4float %222 %221 None
        %224 = OpCompositeExtract %float %177 3
        %225 = OpFSub %float %224 %float_24
        %226 = OpFMul %float %225 %float_0_00066666666
        %227 = OpExtInst %float %1 FMax %226 %float_0
        %228 = OpExtInst %float %1 FMin %227 %float_1
        %229 = OpCompositeExtract %float %223 1
        %230 = OpExtInst %float %1 FMix %229 %float_1 %228
        %231 = OpExtInst %float %1 FMix %219 %220 %230
        %232 = OpSampledImage %type_sampled_image_0 %214 %215
        %233 = OpImageSampleImplicitLod %v4float %232 %182 None
        %234 = OpExtInst %float %1 FMix %229 %float_0 %228
        %235 = OpCompositeExtract %float %233 1
        %236 = OpFAdd %float %235 %234
        %237 = OpExtInst %float %1 FMix %236 %float_0_5 %float_0_5
        %238 = OpExtInst %float %1 FMix %float_0_294999987 %float_0_660000026 %237
        %239 = OpFMul %float %238 %float_0_5
        %240 = OpFMul %float %231 %239
        %241 = OpExtInst %float %1 FMix %float_0 %float_0_5 %235
        %242 = OpExtInst %float %1 FMix %float_0_699999988 %float_1 %229
        %243 = OpExtInst %float %1 FMix %242 %float_1 %228
        %244 = OpFAdd %float %241 %243
        %245 = OpExtInst %float %1 FMax %244 %float_0
        %246 = OpExtInst %float %1 FMin %245 %float_1
        %247 = OpCompositeConstruct %v3float %240 %240 %240
        %248 = OpExtInst %v3float %1 FClamp %247 %47 %64
        %249 = OpCompositeExtract %float %158 1
        %250 = OpFMul %float %246 %249
        %251 = OpCompositeExtract %float %158 0
        %252 = OpFAdd %float %250 %251
        %253 = OpExtInst %float %1 FClamp %252 %float_0_119999997 %float_1
        %254 = OpExtInst %float %1 FMax %208 %float_0
        %255 = OpCompositeConstruct %v2float %253 %253
        %256 = OpFMul %v2float %255 %68
        %257 = OpFAdd %v2float %256 %70
        %258 = OpCompositeExtract %float %257 0
        %259 = OpFMul %float %258 %258
        %260 = OpFMul %float %float_n9_27999973 %254
        %261 = OpExtInst %float %1 Exp2 %260
        %262 = OpExtInst %float %1 FMin %259 %261
        %263 = OpFMul %float %262 %258
        %264 = OpCompositeExtract %float %257 1
        %265 = OpFAdd %float %263 %264
        %266 = OpCompositeExtract %float %152 3
        %267 = OpCompositeConstruct %v3float %266 %266 %266
        %268 = OpFMul %v3float %248 %267
        %269 = OpVectorShuffle %v3float %152 %152 0 1 2
        %270 = OpFAdd %v3float %268 %269
        %271 = OpCompositeExtract %float %154 3
        %272 = OpFMul %float %265 %271
        %273 = OpCompositeConstruct %v3float %272 %272 %272
        %274 = OpVectorShuffle %v3float %154 %154 0 1 2
        %275 = OpFAdd %v3float %273 %274
        %276 = OpCompositeExtract %float %275 0
        %277 = OpExtInst %float %1 FClamp %float_1 %float_0 %float_1
        %278 = OpLoad %type_2d_image %MobileDirectionalLight_DirectionalLightShadowTexture
        %279 = OpLoad %type_sampler %MobileDirectionalLight_DirectionalLightShadowSampler
        %280 = OpAccessChain %_ptr_Uniform_v4float %MobileDirectionalLight %int_1
        %281 = OpAccessChain %_ptr_Uniform_float %MobileDirectionalLight %int_1 %int_3
        %282 = OpLoad %float %281
        %283 = OpAccessChain %_ptr_Uniform_v4float %MobileDirectionalLight %int_2
        %284 = OpLoad %v4float %283
               OpBranch %285
        %285 = OpLabel
        %286 = OpPhi %int %int_0 %140 %287 %288
        %289 = OpSLessThan %bool %286 %int_2
               OpLoopMerge %290 %288 None
               OpBranchConditional %289 %291 %290
        %291 = OpLabel
        %292 = OpBitcast %uint %286
        %293 = OpAccessChain %_ptr_Uniform_float %MobileDirectionalLight %int_4 %292
        %294 = OpLoad %float %293
        %295 = OpFOrdLessThan %bool %224 %294
               OpSelectionMerge %288 None
               OpBranchConditional %295 %296 %288
        %296 = OpLabel
        %297 = OpCompositeExtract %float %177 0
        %298 = OpCompositeExtract %float %177 1
        %299 = OpCompositeConstruct %v4float %297 %298 %224 %float_1
        %300 = OpAccessChain %_ptr_Uniform_mat4v4float %MobileDirectionalLight %int_5 %286
        %301 = OpLoad %mat4v4float %300
        %302 = OpMatrixTimesVector %v4float %301 %299
               OpBranch %290
        %288 = OpLabel
        %287 = OpIAdd %int %286 %int_1
               OpBranch %285
        %290 = OpLabel
        %303 = OpPhi %v4float %52 %285 %302 %296
        %304 = OpCompositeExtract %float %303 2
        %305 = OpFOrdGreaterThan %bool %304 %float_0
               OpSelectionMerge %306 None
               OpBranchConditional %305 %307 %306
        %307 = OpLabel
        %308 = OpExtInst %float %1 FMin %304 %float_0_999989986
        %309 = OpVectorShuffle %v2float %303 %303 0 1
        %310 = OpVectorShuffle %v2float %284 %284 0 1
        %311 = OpFMul %v2float %309 %310
        %312 = OpExtInst %v2float %1 Fract %311
        %313 = OpExtInst %v2float %1 Floor %311
        %314 = OpFAdd %v2float %313 %127
        %315 = OpVectorShuffle %v2float %284 %284 2 3
        %316 = OpFMul %v2float %314 %315
        %317 = OpSampledImage %type_sampled_image_0 %278 %279
        %318 = OpImageSampleExplicitLod %v4float %317 %316 Lod %float_0
        %319 = OpCompositeExtract %float %318 0
        %320 = OpCompositeInsert %v3float %319 %139 0
        %321 = OpFAdd %v2float %313 %128
        %322 = OpFMul %v2float %321 %315
        %323 = OpSampledImage %type_sampled_image_0 %278 %279
        %324 = OpImageSampleExplicitLod %v4float %323 %322 Lod %float_0
        %325 = OpCompositeExtract %float %324 0
        %326 = OpCompositeInsert %v3float %325 %320 1
        %327 = OpFAdd %v2float %313 %130
        %328 = OpFMul %v2float %327 %315
        %329 = OpSampledImage %type_sampled_image_0 %278 %279
        %330 = OpImageSampleExplicitLod %v4float %329 %328 Lod %float_0
        %331 = OpCompositeExtract %float %330 0
        %332 = OpCompositeInsert %v3float %331 %326 2
        %333 = OpFMul %float %308 %282
        %334 = OpFSub %float %333 %float_1
        %335 = OpCompositeConstruct %v3float %282 %282 %282
        %336 = OpFMul %v3float %332 %335
        %337 = OpCompositeConstruct %v3float %334 %334 %334
        %338 = OpFSub %v3float %336 %337
        %339 = OpExtInst %v3float %1 FClamp %338 %47 %64
        %340 = OpFAdd %v2float %313 %131
        %341 = OpFMul %v2float %340 %315
        %342 = OpSampledImage %type_sampled_image_0 %278 %279
        %343 = OpImageSampleExplicitLod %v4float %342 %341 Lod %float_0
        %344 = OpCompositeExtract %float %343 0
        %345 = OpCompositeInsert %v3float %344 %139 0
        %346 = OpFAdd %v2float %313 %60
        %347 = OpFMul %v2float %346 %315
        %348 = OpSampledImage %type_sampled_image_0 %278 %279
        %349 = OpImageSampleExplicitLod %v4float %348 %347 Lod %float_0
        %350 = OpCompositeExtract %float %349 0
        %351 = OpCompositeInsert %v3float %350 %345 1
        %352 = OpFAdd %v2float %313 %132
        %353 = OpFMul %v2float %352 %315
        %354 = OpSampledImage %type_sampled_image_0 %278 %279
        %355 = OpImageSampleExplicitLod %v4float %354 %353 Lod %float_0
        %356 = OpCompositeExtract %float %355 0
        %357 = OpCompositeInsert %v3float %356 %351 2
        %358 = OpFMul %v3float %357 %335
        %359 = OpFSub %v3float %358 %337
        %360 = OpExtInst %v3float %1 FClamp %359 %47 %64
        %361 = OpFAdd %v2float %313 %133
        %362 = OpFMul %v2float %361 %315
        %363 = OpSampledImage %type_sampled_image_0 %278 %279
        %364 = OpImageSampleExplicitLod %v4float %363 %362 Lod %float_0
        %365 = OpCompositeExtract %float %364 0
        %366 = OpCompositeInsert %v3float %365 %139 0
        %367 = OpFAdd %v2float %313 %134
        %368 = OpFMul %v2float %367 %315
        %369 = OpSampledImage %type_sampled_image_0 %278 %279
        %370 = OpImageSampleExplicitLod %v4float %369 %368 Lod %float_0
        %371 = OpCompositeExtract %float %370 0
        %372 = OpCompositeInsert %v3float %371 %366 1
        %373 = OpFAdd %v2float %313 %135
        %374 = OpFMul %v2float %373 %315
        %375 = OpSampledImage %type_sampled_image_0 %278 %279
        %376 = OpImageSampleExplicitLod %v4float %375 %374 Lod %float_0
        %377 = OpCompositeExtract %float %376 0
        %378 = OpCompositeInsert %v3float %377 %372 2
        %379 = OpFMul %v3float %378 %335
        %380 = OpFSub %v3float %379 %337
        %381 = OpExtInst %v3float %1 FClamp %380 %47 %64
        %382 = OpCompositeExtract %float %339 0
        %383 = OpCompositeExtract %float %312 0
        %384 = OpFSub %float %float_1 %383
        %385 = OpFMul %float %382 %384
        %386 = OpCompositeExtract %float %360 0
        %387 = OpFMul %float %386 %384
        %388 = OpCompositeExtract %float %381 0
        %389 = OpFMul %float %388 %384
        %390 = OpCompositeExtract %float %339 1
        %391 = OpFAdd %float %385 %390
        %392 = OpCompositeExtract %float %360 1
        %393 = OpFAdd %float %387 %392
        %394 = OpCompositeExtract %float %381 1
        %395 = OpFAdd %float %389 %394
        %396 = OpCompositeExtract %float %339 2
        %397 = OpFMul %float %396 %383
        %398 = OpFAdd %float %391 %397
        %399 = OpCompositeInsert %v3float %398 %136 0
        %400 = OpCompositeExtract %float %360 2
        %401 = OpFMul %float %400 %383
        %402 = OpFAdd %float %393 %401
        %403 = OpCompositeInsert %v3float %402 %399 1
        %404 = OpCompositeExtract %float %381 2
        %405 = OpFMul %float %404 %383
        %406 = OpFAdd %float %395 %405
        %407 = OpCompositeInsert %v3float %406 %403 2
        %408 = OpCompositeExtract %float %312 1
        %409 = OpFSub %float %float_1 %408
        %410 = OpCompositeConstruct %v3float %409 %float_1 %408
        %411 = OpDot %float %407 %410
        %412 = OpFMul %float %float_0_25 %411
        %413 = OpExtInst %float %1 FClamp %412 %float_0 %float_1
        %414 = OpAccessChain %_ptr_Uniform_float %MobileDirectionalLight %int_3 %int_0
        %415 = OpLoad %float %414
        %416 = OpFMul %float %224 %415
        %417 = OpAccessChain %_ptr_Uniform_float %MobileDirectionalLight %int_3 %int_1
        %418 = OpLoad %float %417
        %419 = OpFAdd %float %416 %418
        %420 = OpExtInst %float %1 FClamp %419 %float_0 %float_1
        %421 = OpFMul %float %420 %420
        %422 = OpExtInst %float %1 FMix %413 %float_1 %421
               OpBranch %306
        %306 = OpLabel
        %423 = OpPhi %float %float_1 %290 %422 %307
        %424 = OpLoad %v4float %280
        %425 = OpVectorShuffle %v3float %424 %424 0 1 2
        %426 = OpDot %float %206 %425
        %427 = OpExtInst %float %1 FMax %float_0 %426
        %428 = OpFAdd %v3float %181 %425
        %429 = OpExtInst %v3float %1 Normalize %428
        %430 = OpDot %float %206 %429
        %431 = OpExtInst %float %1 FMax %float_0 %430
        %432 = OpFMul %float %423 %427
        %433 = OpCompositeConstruct %v3float %432 %432 %432
        %434 = OpAccessChain %_ptr_Uniform_v4float %MobileDirectionalLight %int_0
        %435 = OpLoad %v4float %434
        %436 = OpVectorShuffle %v3float %435 %435 0 1 2
        %437 = OpFMul %v3float %433 %436
        %438 = OpFMul %float %253 %float_0_25
        %439 = OpFAdd %float %438 %float_0_25
        %440 = OpExtInst %v3float %1 Cross %206 %429
        %441 = OpDot %float %440 %440
        %442 = OpFMul %float %253 %253
        %443 = OpFMul %float %431 %442
        %444 = OpFMul %float %443 %443
        %445 = OpFAdd %float %441 %444
        %446 = OpFDiv %float %442 %445
        %447 = OpFMul %float %446 %446
        %448 = OpExtInst %float %1 FMin %447 %float_65504
        %449 = OpFMul %float %439 %448
        %450 = OpFMul %float %276 %449
        %451 = OpCompositeConstruct %v3float %450 %450 %450
        %452 = OpFAdd %v3float %270 %451
        %453 = OpFMul %v3float %437 %452
        %454 = OpAccessChain %_ptr_Uniform_float %_Globals %int_3 %int_3
        %455 = OpLoad %float %454
        %456 = OpFOrdGreaterThan %bool %455 %float_0
        %457 = OpSelect %float %456 %float_1 %float_0
        %458 = OpFOrdNotEqual %bool %457 %float_0
        %459 = OpSelect %float %458 %455 %164
        %460 = OpExtInst %float %1 Log2 %253
        %461 = OpFMul %float %float_1_20000005 %460
        %462 = OpFSub %float %float_1 %461
        %463 = OpFSub %float %459 %float_1
        %464 = OpFSub %float %463 %462
        %465 = OpLoad %type_cube_image %ReflectionCubemap
        %466 = OpLoad %type_sampler %ReflectionCubemapSampler
        %467 = OpSampledImage %type_sampled_image %465 %466
        %468 = OpImageSampleExplicitLod %v4float %467 %212 Lod %464
               OpSelectionMerge %469 None
               OpBranchConditional %458 %470 %471
        %471 = OpLabel
        %472 = OpVectorShuffle %v3float %468 %468 0 1 2
        %473 = OpCompositeExtract %float %468 3
        %474 = OpFMul %float %473 %float_16
        %475 = OpCompositeConstruct %v3float %474 %474 %474
        %476 = OpFMul %v3float %472 %475
        %477 = OpFMul %v3float %476 %476
               OpBranch %469
        %470 = OpLabel
        %478 = OpVectorShuffle %v3float %468 %468 0 1 2
        %479 = OpVectorShuffle %v3float %162 %162 0 1 2
        %480 = OpFMul %v3float %478 %479
               OpBranch %469
        %469 = OpLabel
        %481 = OpPhi %v3float %477 %471 %480 %470
        %482 = OpCompositeConstruct %v3float %277 %277 %277
        %483 = OpFMul %v3float %481 %482
        %484 = OpCompositeConstruct %v3float %276 %276 %276
        %485 = OpFMul %v3float %483 %484
        %486 = OpFAdd %v3float %453 %485
               OpBranch %487
        %487 = OpLabel
        %488 = OpPhi %v3float %486 %469 %489 %490
        %491 = OpPhi %int %int_0 %469 %492 %490
        %493 = OpAccessChain %_ptr_Uniform_int %_Globals %int_0
        %494 = OpLoad %int %493
        %495 = OpSLessThan %bool %491 %494
               OpLoopMerge %496 %490 None
               OpBranchConditional %495 %497 %496
        %497 = OpLabel
        %498 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_1 %491
        %499 = OpLoad %v4float %498
        %500 = OpVectorShuffle %v3float %499 %499 0 1 2
        %501 = OpFSub %v3float %500 %179
        %502 = OpDot %float %501 %501
        %503 = OpExtInst %float %1 InverseSqrt %502
        %504 = OpCompositeConstruct %v3float %503 %503 %503
        %505 = OpFMul %v3float %501 %504
        %506 = OpFAdd %v3float %181 %505
        %507 = OpExtInst %v3float %1 Normalize %506
        %508 = OpDot %float %206 %505
        %509 = OpExtInst %float %1 FMax %float_0 %508
        %510 = OpDot %float %206 %507
        %511 = OpExtInst %float %1 FMax %float_0 %510
        %512 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_2 %491
        %513 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2 %491 %int_3
        %514 = OpLoad %float %513
        %515 = OpFOrdEqual %bool %514 %float_0
               OpSelectionMerge %490 None
               OpBranchConditional %515 %516 %517
        %517 = OpLabel
        %518 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %491 %int_3
        %519 = OpLoad %float %518
        %520 = OpCompositeConstruct %v3float %519 %519 %519
        %521 = OpFMul %v3float %501 %520
        %522 = OpDot %float %521 %521
        %523 = OpExtInst %float %1 FClamp %522 %float_0 %float_1
        %524 = OpFSub %float %float_1 %523
        %525 = OpExtInst %float %1 Pow %524 %514
               OpBranch %490
        %516 = OpLabel
        %526 = OpFAdd %float %502 %float_1
        %527 = OpFDiv %float %float_1 %526
        %528 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %491 %int_3
        %529 = OpLoad %float %528
        %530 = OpFMul %float %529 %529
        %531 = OpFMul %float %502 %530
        %532 = OpFMul %float %531 %531
        %533 = OpFSub %float %float_1 %532
        %534 = OpExtInst %float %1 FClamp %533 %float_0 %float_1
        %535 = OpFMul %float %534 %534
        %536 = OpFMul %float %527 %535
               OpBranch %490
        %490 = OpLabel
        %537 = OpPhi %float %525 %517 %536 %516
        %538 = OpFMul %float %537 %509
        %539 = OpCompositeConstruct %v3float %538 %538 %538
        %540 = OpLoad %v4float %512
        %541 = OpVectorShuffle %v3float %540 %540 0 1 2
        %542 = OpFMul %v3float %539 %541
        %543 = OpFMul %v3float %542 %57
        %544 = OpExtInst %v3float %1 Cross %206 %507
        %545 = OpDot %float %544 %544
        %546 = OpFMul %float %511 %442
        %547 = OpFMul %float %546 %546
        %548 = OpFAdd %float %545 %547
        %549 = OpFDiv %float %442 %548
        %550 = OpFMul %float %549 %549
        %551 = OpExtInst %float %1 FMin %550 %float_65504
        %552 = OpFMul %float %439 %551
        %553 = OpFMul %float %276 %552
        %554 = OpCompositeConstruct %v3float %553 %553 %553
        %555 = OpFAdd %v3float %270 %554
        %556 = OpFMul %v3float %543 %555
        %557 = OpExtInst %v3float %1 FMin %55 %556
        %489 = OpFAdd %v3float %488 %557
        %492 = OpIAdd %int %491 %int_1
               OpBranch %487
        %496 = OpLabel
        %558 = OpExtInst %v3float %1 FMax %47 %47
        %559 = OpFAdd %v3float %488 %558
        %560 = OpFAdd %v3float %270 %484
        %561 = OpCompositeConstruct %v3float %160 %160 %160
        %562 = OpExtInst %v3float %1 FMix %559 %560 %561
        %563 = OpCompositeExtract %float %142 3
        %564 = OpCompositeConstruct %v3float %563 %563 %563
        %565 = OpFMul %v3float %562 %564
        %566 = OpVectorShuffle %v3float %142 %142 0 1 2
        %567 = OpFAdd %v3float %565 %566
        %568 = OpVectorShuffle %v4float %137 %567 4 5 6 3
        %569 = OpCompositeExtract %float %143 3
        %570 = OpExtInst %float %1 FMin %569 %float_65500
        %571 = OpCompositeInsert %v4float %570 %568 3
               OpStore %out_var_SV_Target0 %571
               OpReturn
               OpFunctionEnd
