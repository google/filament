; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 487
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %Main "main" %in_var_ATTRIBUTE0 %in_var_ATTRIBUTE1 %out_var_TEXCOORD0 %out_var_TEXCOORD1 %out_var_TEXCOORD2 %out_var_TEXCOORD3 %out_var_TEXCOORD8 %gl_Position
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
               OpName %type_MobileBasePass "type.MobileBasePass"
               OpMemberName %type_MobileBasePass 0 "MobileBasePass_Fog_ExponentialFogParameters"
               OpMemberName %type_MobileBasePass 1 "MobileBasePass_Fog_ExponentialFogParameters2"
               OpMemberName %type_MobileBasePass 2 "MobileBasePass_Fog_ExponentialFogColorParameter"
               OpMemberName %type_MobileBasePass 3 "MobileBasePass_Fog_ExponentialFogParameters3"
               OpMemberName %type_MobileBasePass 4 "MobileBasePass_Fog_InscatteringLightDirection"
               OpMemberName %type_MobileBasePass 5 "MobileBasePass_Fog_DirectionalInscatteringColor"
               OpMemberName %type_MobileBasePass 6 "MobileBasePass_Fog_SinCosInscatteringColorCubemapRotation"
               OpMemberName %type_MobileBasePass 7 "PrePadding_MobileBasePass_Fog_104"
               OpMemberName %type_MobileBasePass 8 "PrePadding_MobileBasePass_Fog_108"
               OpMemberName %type_MobileBasePass 9 "MobileBasePass_Fog_FogInscatteringTextureParameters"
               OpMemberName %type_MobileBasePass 10 "MobileBasePass_Fog_ApplyVolumetricFog"
               OpMemberName %type_MobileBasePass 11 "PrePadding_MobileBasePass_PlanarReflection_128"
               OpMemberName %type_MobileBasePass 12 "PrePadding_MobileBasePass_PlanarReflection_132"
               OpMemberName %type_MobileBasePass 13 "PrePadding_MobileBasePass_PlanarReflection_136"
               OpMemberName %type_MobileBasePass 14 "PrePadding_MobileBasePass_PlanarReflection_140"
               OpMemberName %type_MobileBasePass 15 "PrePadding_MobileBasePass_PlanarReflection_144"
               OpMemberName %type_MobileBasePass 16 "PrePadding_MobileBasePass_PlanarReflection_148"
               OpMemberName %type_MobileBasePass 17 "PrePadding_MobileBasePass_PlanarReflection_152"
               OpMemberName %type_MobileBasePass 18 "PrePadding_MobileBasePass_PlanarReflection_156"
               OpMemberName %type_MobileBasePass 19 "MobileBasePass_PlanarReflection_ReflectionPlane"
               OpMemberName %type_MobileBasePass 20 "MobileBasePass_PlanarReflection_PlanarReflectionOrigin"
               OpMemberName %type_MobileBasePass 21 "MobileBasePass_PlanarReflection_PlanarReflectionXAxis"
               OpMemberName %type_MobileBasePass 22 "MobileBasePass_PlanarReflection_PlanarReflectionYAxis"
               OpMemberName %type_MobileBasePass 23 "MobileBasePass_PlanarReflection_InverseTransposeMirrorMatrix"
               OpMemberName %type_MobileBasePass 24 "MobileBasePass_PlanarReflection_PlanarReflectionParameters"
               OpMemberName %type_MobileBasePass 25 "PrePadding_MobileBasePass_PlanarReflection_284"
               OpMemberName %type_MobileBasePass 26 "MobileBasePass_PlanarReflection_PlanarReflectionParameters2"
               OpMemberName %type_MobileBasePass 27 "PrePadding_MobileBasePass_PlanarReflection_296"
               OpMemberName %type_MobileBasePass 28 "PrePadding_MobileBasePass_PlanarReflection_300"
               OpMemberName %type_MobileBasePass 29 "MobileBasePass_PlanarReflection_ProjectionWithExtraFOV"
               OpMemberName %type_MobileBasePass 30 "MobileBasePass_PlanarReflection_PlanarReflectionScreenScaleBias"
               OpMemberName %type_MobileBasePass 31 "MobileBasePass_PlanarReflection_PlanarReflectionScreenBound"
               OpMemberName %type_MobileBasePass 32 "MobileBasePass_PlanarReflection_bIsStereo"
               OpName %MobileBasePass "MobileBasePass"
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
               OpMemberName %type_Primitive 13 "Primitive_UseEditorDepthTest"
               OpMemberName %type_Primitive 14 "Primitive_ObjectOrientation"
               OpMemberName %type_Primitive 15 "Primitive_NonUniformScale"
               OpMemberName %type_Primitive 16 "Primitive_LocalObjectBoundsMin"
               OpMemberName %type_Primitive 17 "PrePadding_Primitive_380"
               OpMemberName %type_Primitive 18 "Primitive_LocalObjectBoundsMax"
               OpMemberName %type_Primitive 19 "Primitive_LightingChannelMask"
               OpMemberName %type_Primitive 20 "Primitive_LightmapDataIndex"
               OpMemberName %type_Primitive 21 "Primitive_SingleCaptureIndex"
               OpName %Primitive "Primitive"
               OpName %type_LandscapeParameters "type.LandscapeParameters"
               OpMemberName %type_LandscapeParameters 0 "LandscapeParameters_HeightmapUVScaleBias"
               OpMemberName %type_LandscapeParameters 1 "LandscapeParameters_WeightmapUVScaleBias"
               OpMemberName %type_LandscapeParameters 2 "LandscapeParameters_LandscapeLightmapScaleBias"
               OpMemberName %type_LandscapeParameters 3 "LandscapeParameters_SubsectionSizeVertsLayerUVPan"
               OpMemberName %type_LandscapeParameters 4 "LandscapeParameters_SubsectionOffsetParams"
               OpMemberName %type_LandscapeParameters 5 "LandscapeParameters_LightmapSubsectionOffsetParams"
               OpMemberName %type_LandscapeParameters 6 "LandscapeParameters_LocalToWorldNoScaling"
               OpName %LandscapeParameters "LandscapeParameters"
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "LodBias"
               OpMemberName %type__Globals 1 "LodValues"
               OpMemberName %type__Globals 2 "SectionLods"
               OpMemberName %type__Globals 3 "NeighborSectionLod"
               OpName %_Globals "$Globals"
               OpName %in_var_ATTRIBUTE0 "in.var.ATTRIBUTE0"
               OpName %in_var_ATTRIBUTE1 "in.var.ATTRIBUTE1"
               OpName %out_var_TEXCOORD0 "out.var.TEXCOORD0"
               OpName %out_var_TEXCOORD1 "out.var.TEXCOORD1"
               OpName %out_var_TEXCOORD2 "out.var.TEXCOORD2"
               OpName %out_var_TEXCOORD3 "out.var.TEXCOORD3"
               OpName %out_var_TEXCOORD8 "out.var.TEXCOORD8"
               OpName %Main "Main"
               OpDecorateString %in_var_ATTRIBUTE0 UserSemantic "ATTRIBUTE0"
               OpDecorateString %in_var_ATTRIBUTE1 UserSemantic "ATTRIBUTE1"
               OpDecorateString %out_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %out_var_TEXCOORD1 UserSemantic "TEXCOORD1"
               OpDecorateString %out_var_TEXCOORD2 UserSemantic "TEXCOORD2"
               OpDecorateString %out_var_TEXCOORD3 UserSemantic "TEXCOORD3"
               OpDecorateString %out_var_TEXCOORD8 UserSemantic "TEXCOORD8"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_POSITION"
               OpDecorate %in_var_ATTRIBUTE0 Location 0
               OpDecorate %in_var_ATTRIBUTE1 Location 1
               OpDecorate %out_var_TEXCOORD0 Location 0
               OpDecorate %out_var_TEXCOORD1 Location 1
               OpDecorate %out_var_TEXCOORD2 Location 2
               OpDecorate %out_var_TEXCOORD3 Location 3
               OpDecorate %out_var_TEXCOORD8 Location 4
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 0
               OpDecorate %MobileBasePass DescriptorSet 0
               OpDecorate %MobileBasePass Binding 1
               OpDecorate %Primitive DescriptorSet 0
               OpDecorate %Primitive Binding 2
               OpDecorate %LandscapeParameters DescriptorSet 0
               OpDecorate %LandscapeParameters Binding 3
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 4
               OpDecorate %_arr_v4float_uint_2_0 ArrayStride 16
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
               OpDecorate %_arr_mat4v4float_uint_2 ArrayStride 64
               OpMemberDecorate %type_MobileBasePass 0 Offset 0
               OpMemberDecorate %type_MobileBasePass 1 Offset 16
               OpMemberDecorate %type_MobileBasePass 2 Offset 32
               OpMemberDecorate %type_MobileBasePass 3 Offset 48
               OpMemberDecorate %type_MobileBasePass 4 Offset 64
               OpMemberDecorate %type_MobileBasePass 5 Offset 80
               OpMemberDecorate %type_MobileBasePass 6 Offset 96
               OpMemberDecorate %type_MobileBasePass 7 Offset 104
               OpMemberDecorate %type_MobileBasePass 8 Offset 108
               OpMemberDecorate %type_MobileBasePass 9 Offset 112
               OpMemberDecorate %type_MobileBasePass 10 Offset 124
               OpMemberDecorate %type_MobileBasePass 11 Offset 128
               OpMemberDecorate %type_MobileBasePass 12 Offset 132
               OpMemberDecorate %type_MobileBasePass 13 Offset 136
               OpMemberDecorate %type_MobileBasePass 14 Offset 140
               OpMemberDecorate %type_MobileBasePass 15 Offset 144
               OpMemberDecorate %type_MobileBasePass 16 Offset 148
               OpMemberDecorate %type_MobileBasePass 17 Offset 152
               OpMemberDecorate %type_MobileBasePass 18 Offset 156
               OpMemberDecorate %type_MobileBasePass 19 Offset 160
               OpMemberDecorate %type_MobileBasePass 20 Offset 176
               OpMemberDecorate %type_MobileBasePass 21 Offset 192
               OpMemberDecorate %type_MobileBasePass 22 Offset 208
               OpMemberDecorate %type_MobileBasePass 23 Offset 224
               OpMemberDecorate %type_MobileBasePass 23 MatrixStride 16
               OpMemberDecorate %type_MobileBasePass 23 ColMajor
               OpMemberDecorate %type_MobileBasePass 24 Offset 272
               OpMemberDecorate %type_MobileBasePass 25 Offset 284
               OpMemberDecorate %type_MobileBasePass 26 Offset 288
               OpMemberDecorate %type_MobileBasePass 27 Offset 296
               OpMemberDecorate %type_MobileBasePass 28 Offset 300
               OpMemberDecorate %type_MobileBasePass 29 Offset 304
               OpMemberDecorate %type_MobileBasePass 29 MatrixStride 16
               OpMemberDecorate %type_MobileBasePass 29 ColMajor
               OpMemberDecorate %type_MobileBasePass 30 Offset 432
               OpMemberDecorate %type_MobileBasePass 31 Offset 464
               OpMemberDecorate %type_MobileBasePass 32 Offset 472
               OpDecorate %type_MobileBasePass Block
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
               OpMemberDecorate %type_Primitive 21 Offset 404
               OpDecorate %type_Primitive Block
               OpMemberDecorate %type_LandscapeParameters 0 Offset 0
               OpMemberDecorate %type_LandscapeParameters 1 Offset 16
               OpMemberDecorate %type_LandscapeParameters 2 Offset 32
               OpMemberDecorate %type_LandscapeParameters 3 Offset 48
               OpMemberDecorate %type_LandscapeParameters 4 Offset 64
               OpMemberDecorate %type_LandscapeParameters 5 Offset 80
               OpMemberDecorate %type_LandscapeParameters 6 Offset 96
               OpMemberDecorate %type_LandscapeParameters 6 MatrixStride 16
               OpMemberDecorate %type_LandscapeParameters 6 ColMajor
               OpDecorate %type_LandscapeParameters Block
               OpMemberDecorate %type__Globals 0 Offset 0
               OpMemberDecorate %type__Globals 1 Offset 16
               OpMemberDecorate %type__Globals 2 Offset 32
               OpMemberDecorate %type__Globals 3 Offset 48
               OpDecorate %type__Globals Block
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%mat4v4float = OpTypeMatrix %v4float 4
    %v3float = OpTypeVector %float 3
    %v2float = OpTypeVector %float 2
        %int = OpTypeInt 32 1
       %uint = OpTypeInt 32 0
     %uint_2 = OpConstant %uint 2
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
     %uint_7 = OpConstant %uint 7
     %uint_4 = OpConstant %uint 4
%float_0_00999999978 = OpConstant %float 0.00999999978
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_0 = OpConstant %float 0
         %40 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
  %float_255 = OpConstant %float 255
         %44 = OpConstantComposite %v4float %float_255 %float_255 %float_255 %float_255
  %float_0_5 = OpConstant %float 0.5
         %46 = OpConstantComposite %v2float %float_0_5 %float_0_5
    %float_2 = OpConstant %float 2
         %48 = OpConstantComposite %v2float %float_2 %float_2
    %float_1 = OpConstant %float 1
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
    %float_3 = OpConstant %float 3
 %float_0_25 = OpConstant %float 0.25
     %uint_3 = OpConstant %uint 3
    %float_4 = OpConstant %float 4
%float_0_125 = OpConstant %float 0.125
    %float_5 = OpConstant %float 5
%float_0_0625 = OpConstant %float 0.0625
%float_0_03125 = OpConstant %float 0.03125
         %60 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %int_5 = OpConstant %int 5
      %int_4 = OpConstant %int 4
         %63 = OpConstantComposite %v3float %float_0 %float_0 %float_0
     %int_25 = OpConstant %int 25
     %int_27 = OpConstant %int 27
     %int_31 = OpConstant %int 31
         %67 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%float_32768 = OpConstant %float 32768
%_arr_v4float_uint_2_0 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2_0 %_arr_v4float_uint_2_0 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%mat3v4float = OpTypeMatrix %v4float 3
%_arr_mat4v4float_uint_2 = OpTypeArray %mat4v4float %uint_2
%type_MobileBasePass = OpTypeStruct %v4float %v4float %v4float %v4float %v4float %v4float %v2float %float %float %v3float %float %float %float %float %float %float %float %float %float %v4float %v4float %v4float %v4float %mat3v4float %v3float %float %v2float %float %float %_arr_mat4v4float_uint_2 %_arr_v4float_uint_2_0 %v2float %uint
%_ptr_Uniform_type_MobileBasePass = OpTypePointer Uniform %type_MobileBasePass
%type_Primitive = OpTypeStruct %mat4v4float %v4float %v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %float %float %float %float %v4float %v4float %v3float %float %v3float %uint %uint %int
%_ptr_Uniform_type_Primitive = OpTypePointer Uniform %type_Primitive
%type_LandscapeParameters = OpTypeStruct %v4float %v4float %v4float %v4float %v4float %v4float %mat4v4float
%_ptr_Uniform_type_LandscapeParameters = OpTypePointer Uniform %type_LandscapeParameters
%type__Globals = OpTypeStruct %v4float %v4float %v4float %_arr_v4float_uint_4
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input__arr_v4float_uint_2 = OpTypePointer Input %_arr_v4float_uint_2
%_ptr_Output_v2float = OpTypePointer Output %v2float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %80 = OpTypeFunction %void
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%_ptr_Function__arr_v4float_uint_1 = OpTypePointer Function %_arr_v4float_uint_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
       %View = OpVariable %_ptr_Uniform_type_View Uniform
%MobileBasePass = OpVariable %_ptr_Uniform_type_MobileBasePass Uniform
  %Primitive = OpVariable %_ptr_Uniform_type_Primitive Uniform
%LandscapeParameters = OpVariable %_ptr_Uniform_type_LandscapeParameters Uniform
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%in_var_ATTRIBUTE0 = OpVariable %_ptr_Input_v4float Input
%in_var_ATTRIBUTE1 = OpVariable %_ptr_Input__arr_v4float_uint_2 Input
%out_var_TEXCOORD0 = OpVariable %_ptr_Output_v2float Output
%out_var_TEXCOORD1 = OpVariable %_ptr_Output_v2float Output
%out_var_TEXCOORD2 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD3 = OpVariable %_ptr_Output_v4float Output
%out_var_TEXCOORD8 = OpVariable %_ptr_Output_v4float Output
%gl_Position = OpVariable %_ptr_Output_v4float Output
%float_0_0078125 = OpConstant %float 0.0078125
 %float_n127 = OpConstant %float -127
         %92 = OpConstantNull %v4float
%float_0_00392156886 = OpConstant %float 0.00392156886
         %94 = OpConstantComposite %v2float %float_0_00392156886 %float_0_00392156886
%float_65280 = OpConstant %float 65280
       %Main = OpFunction %void None %80
         %96 = OpLabel
         %97 = OpVariable %_ptr_Function__arr_v4float_uint_1 Function
         %98 = OpLoad %v4float %in_var_ATTRIBUTE0
         %99 = OpLoad %_arr_v4float_uint_2 %in_var_ATTRIBUTE1
        %100 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_0
        %101 = OpLoad %mat4v4float %100
        %102 = OpAccessChain %_ptr_Uniform_v3float %View %int_27
        %103 = OpLoad %v3float %102
        %104 = OpAccessChain %_ptr_Uniform_v3float %View %int_31
        %105 = OpLoad %v3float %104
               OpBranch %106
        %106 = OpLabel
        %107 = OpPhi %int %int_0 %96 %108 %109
        %110 = OpSLessThan %bool %107 %int_1
               OpLoopMerge %111 %109 Unroll
               OpBranchConditional %110 %109 %111
        %109 = OpLabel
        %112 = OpAccessChain %_ptr_Function_v4float %97 %107
               OpStore %112 %40
        %108 = OpIAdd %int %107 %int_1
               OpBranch %106
        %111 = OpLabel
        %113 = OpCompositeExtract %v4float %99 0
        %114 = OpCompositeExtract %v4float %99 1
        %115 = OpFMul %v4float %98 %44
        %116 = OpVectorShuffle %v2float %115 %115 2 3
        %117 = OpFMul %v2float %116 %46
        %118 = OpExtInst %v2float %1 Fract %117
        %119 = OpFMul %v2float %118 %48
        %120 = OpFSub %v2float %116 %119
        %121 = OpFMul %v2float %120 %94
        %122 = OpVectorShuffle %v2float %115 %92 0 1
        %123 = OpAccessChain %_ptr_Uniform_float %_Globals %int_1 %int_3
        %124 = OpLoad %float %123
        %125 = OpCompositeConstruct %v2float %124 %124
        %126 = OpFMul %v2float %122 %125
        %127 = OpCompositeExtract %float %126 1
        %128 = OpCompositeExtract %float %126 0
        %129 = OpFSub %float %float_1 %128
        %130 = OpFSub %float %float_1 %127
        %131 = OpCompositeConstruct %v4float %127 %128 %129 %130
        %132 = OpFMul %v4float %131 %67
        %133 = OpCompositeExtract %float %119 1
        %134 = OpFOrdGreaterThan %bool %133 %float_0_5
               OpSelectionMerge %135 None
               OpBranchConditional %134 %136 %137
        %136 = OpLabel
        %138 = OpCompositeExtract %float %119 0
        %139 = OpFOrdGreaterThan %bool %138 %float_0_5
               OpSelectionMerge %140 None
               OpBranchConditional %139 %141 %142
        %141 = OpLabel
        %143 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2 %int_3
        %144 = OpLoad %float %143
        %145 = OpCompositeConstruct %v4float %144 %144 %144 %144
        %146 = OpFMul %v4float %132 %145
        %147 = OpFSub %v4float %60 %132
        %148 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_3 %int_3
        %149 = OpLoad %v4float %148
        %150 = OpFMul %v4float %147 %149
        %151 = OpFAdd %v4float %146 %150
               OpBranch %140
        %142 = OpLabel
        %152 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2 %int_2
        %153 = OpLoad %float %152
        %154 = OpCompositeConstruct %v4float %153 %153 %153 %153
        %155 = OpFMul %v4float %132 %154
        %156 = OpFSub %v4float %60 %132
        %157 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_3 %int_2
        %158 = OpLoad %v4float %157
        %159 = OpFMul %v4float %156 %158
        %160 = OpFAdd %v4float %155 %159
               OpBranch %140
        %140 = OpLabel
        %161 = OpPhi %v4float %151 %141 %160 %142
               OpBranch %135
        %137 = OpLabel
        %162 = OpCompositeExtract %float %119 0
        %163 = OpFOrdGreaterThan %bool %162 %float_0_5
               OpSelectionMerge %164 None
               OpBranchConditional %163 %165 %166
        %165 = OpLabel
        %167 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2 %int_1
        %168 = OpLoad %float %167
        %169 = OpCompositeConstruct %v4float %168 %168 %168 %168
        %170 = OpFMul %v4float %132 %169
        %171 = OpFSub %v4float %60 %132
        %172 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_3 %int_1
        %173 = OpLoad %v4float %172
        %174 = OpFMul %v4float %171 %173
        %175 = OpFAdd %v4float %170 %174
               OpBranch %164
        %166 = OpLabel
        %176 = OpAccessChain %_ptr_Uniform_float %_Globals %int_2 %int_0
        %177 = OpLoad %float %176
        %178 = OpCompositeConstruct %v4float %177 %177 %177 %177
        %179 = OpFMul %v4float %132 %178
        %180 = OpFSub %v4float %60 %132
        %181 = OpAccessChain %_ptr_Uniform_v4float %_Globals %int_3 %int_0
        %182 = OpLoad %v4float %181
        %183 = OpFMul %v4float %180 %182
        %184 = OpFAdd %v4float %179 %183
               OpBranch %164
        %164 = OpLabel
        %185 = OpPhi %v4float %175 %165 %184 %166
               OpBranch %135
        %135 = OpLabel
        %186 = OpPhi %v4float %161 %140 %185 %164
        %187 = OpFAdd %float %128 %127
        %188 = OpFOrdGreaterThan %bool %187 %float_1
               OpSelectionMerge %189 None
               OpBranchConditional %188 %190 %191
        %190 = OpLabel
        %192 = OpFOrdLessThan %bool %128 %127
               OpSelectionMerge %193 None
               OpBranchConditional %192 %194 %195
        %194 = OpLabel
        %196 = OpCompositeExtract %float %186 3
               OpBranch %193
        %195 = OpLabel
        %197 = OpCompositeExtract %float %186 2
               OpBranch %193
        %193 = OpLabel
        %198 = OpPhi %float %196 %194 %197 %195
               OpBranch %189
        %191 = OpLabel
        %199 = OpFOrdLessThan %bool %128 %127
               OpSelectionMerge %200 None
               OpBranchConditional %199 %201 %202
        %201 = OpLabel
        %203 = OpCompositeExtract %float %186 1
               OpBranch %200
        %202 = OpLabel
        %204 = OpCompositeExtract %float %186 0
               OpBranch %200
        %200 = OpLabel
        %205 = OpPhi %float %203 %201 %204 %202
               OpBranch %189
        %189 = OpLabel
        %206 = OpPhi %float %198 %193 %205 %200
        %207 = OpExtInst %float %1 Floor %206
        %208 = OpFSub %float %206 %207
        %209 = OpFOrdLessThan %bool %207 %float_1
        %210 = OpCompositeExtract %float %114 0
        %211 = OpCompositeExtract %float %114 1
        %212 = OpCompositeConstruct %v3float %float_1 %210 %211
        %213 = OpFOrdLessThan %bool %207 %float_2
        %214 = OpCompositeExtract %float %114 2
        %215 = OpCompositeConstruct %v3float %float_0_5 %211 %214
        %216 = OpFOrdLessThan %bool %207 %float_3
        %217 = OpCompositeExtract %float %114 3
        %218 = OpCompositeConstruct %v3float %float_0_25 %214 %217
        %219 = OpFOrdLessThan %bool %207 %float_4
        %220 = OpCompositeExtract %float %121 0
        %221 = OpCompositeConstruct %v3float %float_0_125 %217 %220
        %222 = OpFOrdLessThan %bool %207 %float_5
        %223 = OpCompositeExtract %float %121 1
        %224 = OpCompositeConstruct %v3float %float_0_0625 %220 %223
        %225 = OpCompositeConstruct %v3float %float_0_03125 %223 %223
        %226 = OpCompositeConstruct %v3bool %222 %222 %222
        %227 = OpSelect %v3float %226 %224 %225
        %228 = OpCompositeConstruct %v3bool %219 %219 %219
        %229 = OpSelect %v3float %228 %221 %227
        %230 = OpCompositeConstruct %v3bool %216 %216 %216
        %231 = OpSelect %v3float %230 %218 %229
        %232 = OpCompositeConstruct %v3bool %213 %213 %213
        %233 = OpSelect %v3float %232 %215 %231
        %234 = OpCompositeConstruct %v3bool %209 %209 %209
        %235 = OpSelect %v3float %234 %212 %233
        %236 = OpCompositeExtract %float %235 0
        %237 = OpCompositeExtract %float %235 1
        %238 = OpCompositeExtract %float %235 2
        %239 = OpCompositeExtract %float %113 0
        %240 = OpFMul %float %239 %float_65280
        %241 = OpCompositeExtract %float %113 1
        %242 = OpFMul %float %241 %float_255
        %243 = OpFAdd %float %240 %242
        %244 = OpFSub %float %243 %float_32768
        %245 = OpFMul %float %244 %float_0_0078125
        %246 = OpCompositeExtract %float %113 2
        %247 = OpFMul %float %246 %float_65280
        %248 = OpCompositeExtract %float %113 3
        %249 = OpFMul %float %248 %float_255
        %250 = OpFAdd %float %247 %249
        %251 = OpFSub %float %250 %float_32768
        %252 = OpFMul %float %251 %float_0_0078125
        %253 = OpExtInst %float %1 FMix %245 %252 %237
        %254 = OpExtInst %float %1 FMix %245 %252 %238
        %255 = OpCompositeConstruct %v2float %236 %236
        %256 = OpFMul %v2float %122 %255
        %257 = OpExtInst %v2float %1 Floor %256
        %258 = OpAccessChain %_ptr_Uniform_v4float %LandscapeParameters %int_3
        %259 = OpAccessChain %_ptr_Uniform_float %LandscapeParameters %int_3 %int_0
        %260 = OpLoad %float %259
        %261 = OpFMul %float %260 %236
        %262 = OpFSub %float %261 %float_1
        %263 = OpFMul %float %260 %float_0_5
        %264 = OpFMul %float %263 %236
        %265 = OpExtInst %float %1 FMax %264 %float_2
        %266 = OpFSub %float %265 %float_1
        %267 = OpCompositeConstruct %v2float %262 %266
        %268 = OpAccessChain %_ptr_Uniform_float %LandscapeParameters %int_3 %int_1
        %269 = OpLoad %float %268
        %270 = OpCompositeConstruct %v2float %269 %269
        %271 = OpFMul %v2float %267 %270
        %272 = OpCompositeExtract %float %271 0
        %273 = OpCompositeConstruct %v2float %272 %272
        %274 = OpFDiv %v2float %257 %273
        %275 = OpFMul %v2float %257 %46
        %276 = OpExtInst %v2float %1 Floor %275
        %277 = OpCompositeExtract %float %271 1
        %278 = OpCompositeConstruct %v2float %277 %277
        %279 = OpFDiv %v2float %276 %278
        %280 = OpCompositeExtract %float %274 0
        %281 = OpCompositeExtract %float %274 1
        %282 = OpCompositeConstruct %v3float %280 %281 %253
        %283 = OpCompositeExtract %float %279 0
        %284 = OpCompositeExtract %float %279 1
        %285 = OpCompositeConstruct %v3float %283 %284 %254
        %286 = OpCompositeConstruct %v3float %208 %208 %208
        %287 = OpExtInst %v3float %1 FMix %282 %285 %286
        %288 = OpVectorShuffle %v2float %119 %92 0 1
        %289 = OpAccessChain %_ptr_Uniform_v4float %LandscapeParameters %int_4
        %290 = OpLoad %v4float %289
        %291 = OpVectorShuffle %v2float %290 %290 3 3
        %292 = OpFMul %v2float %288 %291
        %293 = OpCompositeExtract %float %292 0
        %294 = OpCompositeExtract %float %292 1
        %295 = OpCompositeConstruct %v3float %293 %294 %float_0
        %296 = OpFAdd %v3float %287 %295
        %297 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_0
        %298 = OpLoad %v4float %297
        %299 = OpVectorShuffle %v3float %298 %298 0 1 2
        %300 = OpVectorShuffle %v3float %296 %296 0 0 0
        %301 = OpFMul %v3float %299 %300
        %302 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_1
        %303 = OpLoad %v4float %302
        %304 = OpVectorShuffle %v3float %303 %303 0 1 2
        %305 = OpVectorShuffle %v3float %296 %296 1 1 1
        %306 = OpFMul %v3float %304 %305
        %307 = OpFAdd %v3float %301 %306
        %308 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_2
        %309 = OpLoad %v4float %308
        %310 = OpVectorShuffle %v3float %309 %309 0 1 2
        %311 = OpVectorShuffle %v3float %296 %296 2 2 2
        %312 = OpFMul %v3float %310 %311
        %313 = OpFAdd %v3float %307 %312
        %314 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_3
        %315 = OpLoad %v4float %314
        %316 = OpVectorShuffle %v3float %315 %315 0 1 2
        %317 = OpFAdd %v3float %316 %105
        %318 = OpFAdd %v3float %313 %317
        %319 = OpCompositeExtract %float %318 0
        %320 = OpCompositeExtract %float %318 1
        %321 = OpCompositeExtract %float %318 2
        %322 = OpCompositeConstruct %v4float %319 %320 %321 %float_1
        %323 = OpVectorShuffle %v2float %287 %287 0 1
        %324 = OpLoad %v4float %258
        %325 = OpVectorShuffle %v2float %324 %324 2 3
        %326 = OpFAdd %v2float %323 %325
        %327 = OpFAdd %v2float %326 %292
        %328 = OpAccessChain %_ptr_Uniform_v4float %LandscapeParameters %int_1
        %329 = OpLoad %v4float %328
        %330 = OpVectorShuffle %v2float %329 %329 0 1
        %331 = OpFMul %v2float %323 %330
        %332 = OpVectorShuffle %v2float %329 %329 2 3
        %333 = OpFAdd %v2float %331 %332
        %334 = OpVectorShuffle %v2float %290 %290 2 2
        %335 = OpFMul %v2float %288 %334
        %336 = OpFAdd %v2float %333 %335
        %337 = OpVectorShuffle %v2float %327 %92 0 1
        %338 = OpVectorShuffle %v4float %322 %322 4 5 6 3
        %339 = OpMatrixTimesVector %v4float %101 %338
        %340 = OpVectorShuffle %v3float %322 %92 0 1 2
        %341 = OpFSub %v3float %340 %103
        %342 = OpAccessChain %_ptr_Uniform_v4float %MobileBasePass %int_2
        %343 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_2 %int_3
        %344 = OpLoad %float %343
        %345 = OpDot %float %341 %341
        %346 = OpExtInst %float %1 InverseSqrt %345
        %347 = OpFMul %float %345 %346
        %348 = OpCompositeConstruct %v3float %346 %346 %346
        %349 = OpFMul %v3float %341 %348
        %350 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_0 %int_0
        %351 = OpLoad %float %350
        %352 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_1 %int_0
        %353 = OpLoad %float %352
        %354 = OpCompositeExtract %float %341 2
        %355 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_0 %int_3
        %356 = OpLoad %float %355
        %357 = OpExtInst %float %1 FMax %float_0 %356
        %358 = OpFOrdGreaterThan %bool %357 %float_0
               OpSelectionMerge %359 None
               OpBranchConditional %358 %360 %359
        %360 = OpLabel
        %361 = OpFMul %float %357 %346
        %362 = OpFMul %float %361 %354
        %363 = OpAccessChain %_ptr_Uniform_float %View %int_25 %int_2
        %364 = OpLoad %float %363
        %365 = OpFAdd %float %364 %362
        %366 = OpFSub %float %354 %362
        %367 = OpFSub %float %float_1 %361
        %368 = OpFMul %float %367 %347
        %369 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_0 %int_1
        %370 = OpLoad %float %369
        %371 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_3 %int_1
        %372 = OpLoad %float %371
        %373 = OpFSub %float %365 %372
        %374 = OpFMul %float %370 %373
        %375 = OpExtInst %float %1 FMax %float_n127 %374
        %376 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_3 %int_0
        %377 = OpLoad %float %376
        %378 = OpFNegate %float %375
        %379 = OpExtInst %float %1 Exp2 %378
        %380 = OpFMul %float %377 %379
        %381 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_1 %int_1
        %382 = OpLoad %float %381
        %383 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_1 %int_3
        %384 = OpLoad %float %383
        %385 = OpFSub %float %365 %384
        %386 = OpFMul %float %382 %385
        %387 = OpExtInst %float %1 FMax %float_n127 %386
        %388 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_1 %int_2
        %389 = OpLoad %float %388
        %390 = OpFNegate %float %387
        %391 = OpExtInst %float %1 Exp2 %390
        %392 = OpFMul %float %389 %391
               OpBranch %359
        %359 = OpLabel
        %393 = OpPhi %float %347 %189 %368 %360
        %394 = OpPhi %float %353 %189 %392 %360
        %395 = OpPhi %float %351 %189 %380 %360
        %396 = OpPhi %float %354 %189 %366 %360
        %397 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_0 %int_1
        %398 = OpLoad %float %397
        %399 = OpFMul %float %398 %396
        %400 = OpExtInst %float %1 FMax %float_n127 %399
        %401 = OpFNegate %float %400
        %402 = OpExtInst %float %1 Exp2 %401
        %403 = OpFSub %float %float_1 %402
        %404 = OpFDiv %float %403 %400
        %405 = OpExtInst %float %1 Log %float_2
        %406 = OpFMul %float %405 %405
        %407 = OpFMul %float %float_0_5 %406
        %408 = OpFMul %float %407 %400
        %409 = OpFSub %float %405 %408
        %410 = OpExtInst %float %1 FAbs %400
        %411 = OpFOrdGreaterThan %bool %410 %float_0_00999999978
        %412 = OpSelect %float %411 %404 %409
        %413 = OpFMul %float %395 %412
        %414 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_1 %int_1
        %415 = OpLoad %float %414
        %416 = OpFMul %float %415 %396
        %417 = OpExtInst %float %1 FMax %float_n127 %416
        %418 = OpFNegate %float %417
        %419 = OpExtInst %float %1 Exp2 %418
        %420 = OpFSub %float %float_1 %419
        %421 = OpFDiv %float %420 %417
        %422 = OpFMul %float %407 %417
        %423 = OpFSub %float %405 %422
        %424 = OpExtInst %float %1 FAbs %417
        %425 = OpFOrdGreaterThan %bool %424 %float_0_00999999978
        %426 = OpSelect %float %425 %421 %423
        %427 = OpFMul %float %394 %426
        %428 = OpFAdd %float %413 %427
        %429 = OpFMul %float %428 %393
        %430 = OpLoad %v4float %342
        %431 = OpVectorShuffle %v3float %430 %430 0 1 2
        %432 = OpAccessChain %_ptr_Uniform_v4float %MobileBasePass %int_4
        %433 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_4 %int_3
        %434 = OpLoad %float %433
        %435 = OpFOrdGreaterThanEqual %bool %434 %float_0
               OpSelectionMerge %436 DontFlatten
               OpBranchConditional %435 %437 %436
        %437 = OpLabel
        %438 = OpAccessChain %_ptr_Uniform_v4float %MobileBasePass %int_5
        %439 = OpLoad %v4float %438
        %440 = OpVectorShuffle %v3float %439 %439 0 1 2
        %441 = OpLoad %v4float %432
        %442 = OpVectorShuffle %v3float %441 %441 0 1 2
        %443 = OpDot %float %349 %442
        %444 = OpExtInst %float %1 FClamp %443 %float_0 %float_1
        %445 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_5 %int_3
        %446 = OpLoad %float %445
        %447 = OpExtInst %float %1 Pow %444 %446
        %448 = OpCompositeConstruct %v3float %447 %447 %447
        %449 = OpFMul %v3float %440 %448
        %450 = OpFSub %float %393 %434
        %451 = OpExtInst %float %1 FMax %450 %float_0
        %452 = OpFMul %float %428 %451
        %453 = OpFNegate %float %452
        %454 = OpExtInst %float %1 Exp2 %453
        %455 = OpExtInst %float %1 FClamp %454 %float_0 %float_1
        %456 = OpFSub %float %float_1 %455
        %457 = OpCompositeConstruct %v3float %456 %456 %456
        %458 = OpFMul %v3float %449 %457
               OpBranch %436
        %436 = OpLabel
        %459 = OpPhi %v3float %63 %359 %458 %437
        %460 = OpFNegate %float %429
        %461 = OpExtInst %float %1 Exp2 %460
        %462 = OpExtInst %float %1 FClamp %461 %float_0 %float_1
        %463 = OpExtInst %float %1 FMax %462 %344
        %464 = OpAccessChain %_ptr_Uniform_float %MobileBasePass %int_3 %int_3
        %465 = OpLoad %float %464
        %466 = OpFOrdGreaterThan %bool %465 %float_0
        %467 = OpFOrdGreaterThan %bool %347 %465
        %468 = OpLogicalAnd %bool %466 %467
        %469 = OpCompositeConstruct %v3bool %468 %468 %468
        %470 = OpSelect %v3float %469 %63 %459
        %471 = OpSelect %float %468 %float_1 %463
        %472 = OpFSub %float %float_1 %471
        %473 = OpCompositeConstruct %v3float %472 %472 %472
        %474 = OpFMul %v3float %431 %473
        %475 = OpFAdd %v3float %474 %470
        %476 = OpCompositeExtract %float %475 0
        %477 = OpCompositeExtract %float %475 1
        %478 = OpCompositeExtract %float %475 2
        %479 = OpCompositeConstruct %v4float %476 %477 %478 %471
        %480 = OpAccessChain %_ptr_Function_v4float %97 %int_0
               OpStore %480 %479
        %481 = OpCompositeExtract %float %339 3
        %482 = OpCompositeInsert %v4float %481 %338 3
        %483 = OpLoad %_arr_v4float_uint_1 %97
        %484 = OpCompositeExtract %v4float %483 0
        %485 = OpVectorShuffle %v4float %92 %484 0 1 4 5
        %486 = OpVectorShuffle %v4float %92 %484 0 1 6 7
               OpStore %out_var_TEXCOORD0 %337
               OpStore %out_var_TEXCOORD1 %336
               OpStore %out_var_TEXCOORD2 %485
               OpStore %out_var_TEXCOORD3 %486
               OpStore %out_var_TEXCOORD8 %482
               OpStore %gl_Position %339
               OpReturn
               OpFunctionEnd
