; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 397
; Schema: 0
               OpCapability Shader
               OpCapability SampledBuffer
               OpCapability StorageImageExtendedFormats
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Vertex %Main "main" %gl_VertexIndex %gl_InstanceIndex %in_var_ATTRIBUTE0 %out_var_TEXCOORD6 %gl_Position
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
               OpName %type_MobileShadowDepthPass "type.MobileShadowDepthPass"
               OpMemberName %type_MobileShadowDepthPass 0 "PrePadding_MobileShadowDepthPass_0"
               OpMemberName %type_MobileShadowDepthPass 1 "PrePadding_MobileShadowDepthPass_4"
               OpMemberName %type_MobileShadowDepthPass 2 "PrePadding_MobileShadowDepthPass_8"
               OpMemberName %type_MobileShadowDepthPass 3 "PrePadding_MobileShadowDepthPass_12"
               OpMemberName %type_MobileShadowDepthPass 4 "PrePadding_MobileShadowDepthPass_16"
               OpMemberName %type_MobileShadowDepthPass 5 "PrePadding_MobileShadowDepthPass_20"
               OpMemberName %type_MobileShadowDepthPass 6 "PrePadding_MobileShadowDepthPass_24"
               OpMemberName %type_MobileShadowDepthPass 7 "PrePadding_MobileShadowDepthPass_28"
               OpMemberName %type_MobileShadowDepthPass 8 "PrePadding_MobileShadowDepthPass_32"
               OpMemberName %type_MobileShadowDepthPass 9 "PrePadding_MobileShadowDepthPass_36"
               OpMemberName %type_MobileShadowDepthPass 10 "PrePadding_MobileShadowDepthPass_40"
               OpMemberName %type_MobileShadowDepthPass 11 "PrePadding_MobileShadowDepthPass_44"
               OpMemberName %type_MobileShadowDepthPass 12 "PrePadding_MobileShadowDepthPass_48"
               OpMemberName %type_MobileShadowDepthPass 13 "PrePadding_MobileShadowDepthPass_52"
               OpMemberName %type_MobileShadowDepthPass 14 "PrePadding_MobileShadowDepthPass_56"
               OpMemberName %type_MobileShadowDepthPass 15 "PrePadding_MobileShadowDepthPass_60"
               OpMemberName %type_MobileShadowDepthPass 16 "PrePadding_MobileShadowDepthPass_64"
               OpMemberName %type_MobileShadowDepthPass 17 "PrePadding_MobileShadowDepthPass_68"
               OpMemberName %type_MobileShadowDepthPass 18 "PrePadding_MobileShadowDepthPass_72"
               OpMemberName %type_MobileShadowDepthPass 19 "PrePadding_MobileShadowDepthPass_76"
               OpMemberName %type_MobileShadowDepthPass 20 "MobileShadowDepthPass_ProjectionMatrix"
               OpMemberName %type_MobileShadowDepthPass 21 "MobileShadowDepthPass_ShadowParams"
               OpMemberName %type_MobileShadowDepthPass 22 "MobileShadowDepthPass_bClampToNearPlane"
               OpMemberName %type_MobileShadowDepthPass 23 "PrePadding_MobileShadowDepthPass_156"
               OpMemberName %type_MobileShadowDepthPass 24 "MobileShadowDepthPass_ShadowViewProjectionMatrices"
               OpName %MobileShadowDepthPass "MobileShadowDepthPass"
               OpName %type_EmitterDynamicUniforms "type.EmitterDynamicUniforms"
               OpMemberName %type_EmitterDynamicUniforms 0 "EmitterDynamicUniforms_LocalToWorldScale"
               OpMemberName %type_EmitterDynamicUniforms 1 "EmitterDynamicUniforms_EmitterInstRandom"
               OpMemberName %type_EmitterDynamicUniforms 2 "PrePadding_EmitterDynamicUniforms_12"
               OpMemberName %type_EmitterDynamicUniforms 3 "EmitterDynamicUniforms_AxisLockRight"
               OpMemberName %type_EmitterDynamicUniforms 4 "EmitterDynamicUniforms_AxisLockUp"
               OpMemberName %type_EmitterDynamicUniforms 5 "EmitterDynamicUniforms_DynamicColor"
               OpMemberName %type_EmitterDynamicUniforms 6 "EmitterDynamicUniforms_MacroUVParameters"
               OpName %EmitterDynamicUniforms "EmitterDynamicUniforms"
               OpName %type_EmitterUniforms "type.EmitterUniforms"
               OpMemberName %type_EmitterUniforms 0 "EmitterUniforms_ColorCurve"
               OpMemberName %type_EmitterUniforms 1 "EmitterUniforms_ColorScale"
               OpMemberName %type_EmitterUniforms 2 "EmitterUniforms_ColorBias"
               OpMemberName %type_EmitterUniforms 3 "EmitterUniforms_MiscCurve"
               OpMemberName %type_EmitterUniforms 4 "EmitterUniforms_MiscScale"
               OpMemberName %type_EmitterUniforms 5 "EmitterUniforms_MiscBias"
               OpMemberName %type_EmitterUniforms 6 "EmitterUniforms_SizeBySpeed"
               OpMemberName %type_EmitterUniforms 7 "EmitterUniforms_SubImageSize"
               OpMemberName %type_EmitterUniforms 8 "EmitterUniforms_TangentSelector"
               OpMemberName %type_EmitterUniforms 9 "EmitterUniforms_CameraFacingBlend"
               OpMemberName %type_EmitterUniforms 10 "EmitterUniforms_RemoveHMDRoll"
               OpMemberName %type_EmitterUniforms 11 "EmitterUniforms_RotationRateScale"
               OpMemberName %type_EmitterUniforms 12 "EmitterUniforms_RotationBias"
               OpMemberName %type_EmitterUniforms 13 "EmitterUniforms_CameraMotionBlurAmount"
               OpMemberName %type_EmitterUniforms 14 "PrePadding_EmitterUniforms_172"
               OpMemberName %type_EmitterUniforms 15 "EmitterUniforms_PivotOffset"
               OpName %EmitterUniforms "EmitterUniforms"
               OpName %type_buffer_image "type.buffer.image"
               OpName %ParticleIndices "ParticleIndices"
               OpName %type__Globals "type.$Globals"
               OpMemberName %type__Globals 0 "ParticleIndicesOffset"
               OpName %_Globals "$Globals"
               OpName %type_2d_image "type.2d.image"
               OpName %PositionTexture "PositionTexture"
               OpName %type_sampler "type.sampler"
               OpName %PositionTextureSampler "PositionTextureSampler"
               OpName %VelocityTexture "VelocityTexture"
               OpName %VelocityTextureSampler "VelocityTextureSampler"
               OpName %AttributesTexture "AttributesTexture"
               OpName %AttributesTextureSampler "AttributesTextureSampler"
               OpName %CurveTexture "CurveTexture"
               OpName %CurveTextureSampler "CurveTextureSampler"
               OpName %in_var_ATTRIBUTE0 "in.var.ATTRIBUTE0"
               OpName %out_var_TEXCOORD6 "out.var.TEXCOORD6"
               OpName %Main "Main"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %gl_VertexIndex BuiltIn VertexIndex
               OpDecorateString %gl_VertexIndex UserSemantic "SV_VertexID"
               OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
               OpDecorateString %gl_InstanceIndex UserSemantic "SV_InstanceID"
               OpDecorateString %in_var_ATTRIBUTE0 UserSemantic "ATTRIBUTE0"
               OpDecorateString %out_var_TEXCOORD6 UserSemantic "TEXCOORD6"
               OpDecorate %gl_Position BuiltIn Position
               OpDecorateString %gl_Position UserSemantic "SV_POSITION"
               OpDecorate %in_var_ATTRIBUTE0 Location 0
               OpDecorate %out_var_TEXCOORD6 Location 0
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 1
               OpDecorate %Primitive DescriptorSet 0
               OpDecorate %Primitive Binding 2
               OpDecorate %MobileShadowDepthPass DescriptorSet 0
               OpDecorate %MobileShadowDepthPass Binding 3
               OpDecorate %EmitterDynamicUniforms DescriptorSet 0
               OpDecorate %EmitterDynamicUniforms Binding 4
               OpDecorate %EmitterUniforms DescriptorSet 0
               OpDecorate %EmitterUniforms Binding 5
               OpDecorate %ParticleIndices DescriptorSet 0
               OpDecorate %ParticleIndices Binding 0
               OpDecorate %_Globals DescriptorSet 0
               OpDecorate %_Globals Binding 6
               OpDecorate %PositionTexture DescriptorSet 0
               OpDecorate %PositionTexture Binding 1
               OpDecorate %PositionTextureSampler DescriptorSet 0
               OpDecorate %PositionTextureSampler Binding 0
               OpDecorate %VelocityTexture DescriptorSet 0
               OpDecorate %VelocityTexture Binding 2
               OpDecorate %VelocityTextureSampler DescriptorSet 0
               OpDecorate %VelocityTextureSampler Binding 1
               OpDecorate %AttributesTexture DescriptorSet 0
               OpDecorate %AttributesTexture Binding 3
               OpDecorate %AttributesTextureSampler DescriptorSet 0
               OpDecorate %AttributesTextureSampler Binding 2
               OpDecorate %CurveTexture DescriptorSet 0
               OpDecorate %CurveTexture Binding 4
               OpDecorate %CurveTextureSampler DescriptorSet 0
               OpDecorate %CurveTextureSampler Binding 3
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
               OpDecorate %_arr_mat4v4float_uint_6 ArrayStride 64
               OpMemberDecorate %type_MobileShadowDepthPass 0 Offset 0
               OpMemberDecorate %type_MobileShadowDepthPass 1 Offset 4
               OpMemberDecorate %type_MobileShadowDepthPass 2 Offset 8
               OpMemberDecorate %type_MobileShadowDepthPass 3 Offset 12
               OpMemberDecorate %type_MobileShadowDepthPass 4 Offset 16
               OpMemberDecorate %type_MobileShadowDepthPass 5 Offset 20
               OpMemberDecorate %type_MobileShadowDepthPass 6 Offset 24
               OpMemberDecorate %type_MobileShadowDepthPass 7 Offset 28
               OpMemberDecorate %type_MobileShadowDepthPass 8 Offset 32
               OpMemberDecorate %type_MobileShadowDepthPass 9 Offset 36
               OpMemberDecorate %type_MobileShadowDepthPass 10 Offset 40
               OpMemberDecorate %type_MobileShadowDepthPass 11 Offset 44
               OpMemberDecorate %type_MobileShadowDepthPass 12 Offset 48
               OpMemberDecorate %type_MobileShadowDepthPass 13 Offset 52
               OpMemberDecorate %type_MobileShadowDepthPass 14 Offset 56
               OpMemberDecorate %type_MobileShadowDepthPass 15 Offset 60
               OpMemberDecorate %type_MobileShadowDepthPass 16 Offset 64
               OpMemberDecorate %type_MobileShadowDepthPass 17 Offset 68
               OpMemberDecorate %type_MobileShadowDepthPass 18 Offset 72
               OpMemberDecorate %type_MobileShadowDepthPass 19 Offset 76
               OpMemberDecorate %type_MobileShadowDepthPass 20 Offset 80
               OpMemberDecorate %type_MobileShadowDepthPass 20 MatrixStride 16
               OpMemberDecorate %type_MobileShadowDepthPass 20 ColMajor
               OpMemberDecorate %type_MobileShadowDepthPass 21 Offset 144
               OpMemberDecorate %type_MobileShadowDepthPass 22 Offset 152
               OpMemberDecorate %type_MobileShadowDepthPass 23 Offset 156
               OpMemberDecorate %type_MobileShadowDepthPass 24 Offset 160
               OpMemberDecorate %type_MobileShadowDepthPass 24 MatrixStride 16
               OpMemberDecorate %type_MobileShadowDepthPass 24 ColMajor
               OpDecorate %type_MobileShadowDepthPass Block
               OpMemberDecorate %type_EmitterDynamicUniforms 0 Offset 0
               OpMemberDecorate %type_EmitterDynamicUniforms 1 Offset 8
               OpMemberDecorate %type_EmitterDynamicUniforms 2 Offset 12
               OpMemberDecorate %type_EmitterDynamicUniforms 3 Offset 16
               OpMemberDecorate %type_EmitterDynamicUniforms 4 Offset 32
               OpMemberDecorate %type_EmitterDynamicUniforms 5 Offset 48
               OpMemberDecorate %type_EmitterDynamicUniforms 6 Offset 64
               OpDecorate %type_EmitterDynamicUniforms Block
               OpMemberDecorate %type_EmitterUniforms 0 Offset 0
               OpMemberDecorate %type_EmitterUniforms 1 Offset 16
               OpMemberDecorate %type_EmitterUniforms 2 Offset 32
               OpMemberDecorate %type_EmitterUniforms 3 Offset 48
               OpMemberDecorate %type_EmitterUniforms 4 Offset 64
               OpMemberDecorate %type_EmitterUniforms 5 Offset 80
               OpMemberDecorate %type_EmitterUniforms 6 Offset 96
               OpMemberDecorate %type_EmitterUniforms 7 Offset 112
               OpMemberDecorate %type_EmitterUniforms 8 Offset 128
               OpMemberDecorate %type_EmitterUniforms 9 Offset 144
               OpMemberDecorate %type_EmitterUniforms 10 Offset 156
               OpMemberDecorate %type_EmitterUniforms 11 Offset 160
               OpMemberDecorate %type_EmitterUniforms 12 Offset 164
               OpMemberDecorate %type_EmitterUniforms 13 Offset 168
               OpMemberDecorate %type_EmitterUniforms 14 Offset 172
               OpMemberDecorate %type_EmitterUniforms 15 Offset 176
               OpDecorate %type_EmitterUniforms Block
               OpMemberDecorate %type__Globals 0 Offset 0
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
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %uint_16 = OpConstant %uint 16
      %int_3 = OpConstant %int 3
    %float_1 = OpConstant %float 1
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
         %54 = OpConstantComposite %v3float %float_0 %float_0 %float_9_99999975en05
      %int_2 = OpConstant %int 2
      %int_5 = OpConstant %int 5
      %int_4 = OpConstant %int 4
  %float_0_5 = OpConstant %float 0.5
 %float_n0_5 = OpConstant %float -0.5
    %float_2 = OpConstant %float 2
         %61 = OpConstantComposite %v2float %float_2 %float_2
      %int_6 = OpConstant %int 6
         %63 = OpConstantComposite %v2float %float_1 %float_1
     %int_11 = OpConstant %int 11
     %int_15 = OpConstant %int 15
      %int_8 = OpConstant %int 8
      %int_9 = OpConstant %int 9
     %int_10 = OpConstant %int 10
     %int_12 = OpConstant %int 12
     %uint_0 = OpConstant %uint 0
     %uint_1 = OpConstant %uint 1
%mat3v3float = OpTypeMatrix %v3float 3
     %int_20 = OpConstant %int 20
     %int_22 = OpConstant %int 22
%float_9_99999997en07 = OpConstant %float 9.99999997e-07
     %int_21 = OpConstant %int 21
     %int_17 = OpConstant %int 17
     %int_19 = OpConstant %int 19
     %int_27 = OpConstant %int 27
     %int_31 = OpConstant %int 31
     %uint_3 = OpConstant %uint 3
         %82 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%float_0_00999999978 = OpConstant %float 0.00999999978
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%type_Primitive = OpTypeStruct %mat4v4float %v4float %v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %float %float %float %float %v4float %v4float %v3float %float %v3float %uint %uint %int
%_ptr_Uniform_type_Primitive = OpTypePointer Uniform %type_Primitive
     %uint_6 = OpConstant %uint 6
%_arr_mat4v4float_uint_6 = OpTypeArray %mat4v4float %uint_6
%type_MobileShadowDepthPass = OpTypeStruct %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %mat4v4float %v2float %float %float %_arr_mat4v4float_uint_6
%_ptr_Uniform_type_MobileShadowDepthPass = OpTypePointer Uniform %type_MobileShadowDepthPass
%type_EmitterDynamicUniforms = OpTypeStruct %v2float %float %float %v4float %v4float %v4float %v4float
%_ptr_Uniform_type_EmitterDynamicUniforms = OpTypePointer Uniform %type_EmitterDynamicUniforms
%type_EmitterUniforms = OpTypeStruct %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v4float %v3float %float %float %float %float %float %v2float
%_ptr_Uniform_type_EmitterUniforms = OpTypePointer Uniform %type_EmitterUniforms
%type_buffer_image = OpTypeImage %float Buffer 2 0 0 1 Rg32f
%_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
%type__Globals = OpTypeStruct %uint
%_ptr_Uniform_type__Globals = OpTypePointer Uniform %type__Globals
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Input_v2float = OpTypePointer Input %v2float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %99 = OpTypeFunction %void
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
       %bool = OpTypeBool
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%type_sampled_image = OpTypeSampledImage %type_2d_image
       %View = OpVariable %_ptr_Uniform_type_View Uniform
  %Primitive = OpVariable %_ptr_Uniform_type_Primitive Uniform
%MobileShadowDepthPass = OpVariable %_ptr_Uniform_type_MobileShadowDepthPass Uniform
%EmitterDynamicUniforms = OpVariable %_ptr_Uniform_type_EmitterDynamicUniforms Uniform
%EmitterUniforms = OpVariable %_ptr_Uniform_type_EmitterUniforms Uniform
%ParticleIndices = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
   %_Globals = OpVariable %_ptr_Uniform_type__Globals Uniform
%PositionTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%PositionTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%VelocityTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%VelocityTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%AttributesTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%AttributesTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%CurveTexture = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%CurveTextureSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%gl_VertexIndex = OpVariable %_ptr_Input_uint Input
%gl_InstanceIndex = OpVariable %_ptr_Input_uint Input
%in_var_ATTRIBUTE0 = OpVariable %_ptr_Input_v2float Input
%out_var_TEXCOORD6 = OpVariable %_ptr_Output_float Output
%gl_Position = OpVariable %_ptr_Output_v4float Output
%float_6_28318548 = OpConstant %float 6.28318548
        %108 = OpConstantNull %v3float
       %Main = OpFunction %void None %99
        %109 = OpLabel
        %110 = OpLoad %uint %gl_VertexIndex
        %111 = OpLoad %uint %gl_InstanceIndex
        %112 = OpLoad %v2float %in_var_ATTRIBUTE0
        %113 = OpAccessChain %_ptr_Uniform_v3float %View %int_15
        %114 = OpLoad %v3float %113
        %115 = OpAccessChain %_ptr_Uniform_v3float %View %int_17
        %116 = OpLoad %v3float %115
        %117 = OpAccessChain %_ptr_Uniform_v3float %View %int_19
        %118 = OpLoad %v3float %117
        %119 = OpAccessChain %_ptr_Uniform_v3float %View %int_21
        %120 = OpLoad %v3float %119
        %121 = OpAccessChain %_ptr_Uniform_v3float %View %int_27
        %122 = OpLoad %v3float %121
        %123 = OpAccessChain %_ptr_Uniform_v3float %View %int_31
        %124 = OpLoad %v3float %123
        %125 = OpIMul %uint %111 %uint_16
        %126 = OpUDiv %uint %110 %uint_4
        %127 = OpIAdd %uint %125 %126
        %128 = OpAccessChain %_ptr_Uniform_uint %_Globals %int_0
        %129 = OpLoad %uint %128
        %130 = OpIAdd %uint %129 %127
        %131 = OpLoad %type_buffer_image %ParticleIndices
        %132 = OpImageFetch %v4float %131 %130 None
        %133 = OpVectorShuffle %v2float %132 %132 0 1
        %134 = OpLoad %type_2d_image %PositionTexture
        %135 = OpLoad %type_sampler %PositionTextureSampler
        %136 = OpSampledImage %type_sampled_image %134 %135
        %137 = OpImageSampleExplicitLod %v4float %136 %133 Lod %float_0
        %138 = OpLoad %type_2d_image %VelocityTexture
        %139 = OpLoad %type_sampler %VelocityTextureSampler
        %140 = OpSampledImage %type_sampled_image %138 %139
        %141 = OpImageSampleExplicitLod %v4float %140 %133 Lod %float_0
        %142 = OpLoad %type_2d_image %AttributesTexture
        %143 = OpLoad %type_sampler %AttributesTextureSampler
        %144 = OpSampledImage %type_sampled_image %142 %143
        %145 = OpImageSampleExplicitLod %v4float %144 %133 Lod %float_0
        %146 = OpCompositeExtract %float %137 3
        %147 = OpExtInst %float %1 Step %146 %float_1
        %148 = OpVectorShuffle %v3float %141 %141 0 1 2
        %149 = OpAccessChain %_ptr_Uniform_mat4v4float %Primitive %int_0
        %150 = OpLoad %mat4v4float %149
        %151 = OpCompositeExtract %v4float %150 0
        %152 = OpVectorShuffle %v3float %151 %151 0 1 2
        %153 = OpCompositeExtract %v4float %150 1
        %154 = OpVectorShuffle %v3float %153 %153 0 1 2
        %155 = OpCompositeExtract %v4float %150 2
        %156 = OpVectorShuffle %v3float %155 %155 0 1 2
        %157 = OpCompositeConstruct %mat3v3float %152 %154 %156
        %158 = OpMatrixTimesVector %v3float %157 %148
        %159 = OpFAdd %v3float %158 %54
        %160 = OpExtInst %v3float %1 Normalize %159
        %161 = OpExtInst %float %1 Length %158
        %162 = OpAccessChain %_ptr_Uniform_v4float %EmitterUniforms %int_3
        %163 = OpLoad %v4float %162
        %164 = OpVectorShuffle %v2float %163 %163 0 1
        %165 = OpVectorShuffle %v2float %163 %163 2 3
        %166 = OpCompositeConstruct %v2float %146 %146
        %167 = OpFMul %v2float %165 %166
        %168 = OpFAdd %v2float %164 %167
        %169 = OpLoad %type_2d_image %CurveTexture
        %170 = OpLoad %type_sampler %CurveTextureSampler
        %171 = OpSampledImage %type_sampled_image %169 %170
        %172 = OpImageSampleExplicitLod %v4float %171 %168 Lod %float_0
        %173 = OpAccessChain %_ptr_Uniform_v4float %EmitterUniforms %int_4
        %174 = OpLoad %v4float %173
        %175 = OpFMul %v4float %172 %174
        %176 = OpAccessChain %_ptr_Uniform_v4float %EmitterUniforms %int_5
        %177 = OpLoad %v4float %176
        %178 = OpFAdd %v4float %175 %177
        %179 = OpCompositeExtract %float %145 0
        %180 = OpFOrdLessThan %bool %179 %float_0_5
        %181 = OpSelect %float %180 %float_0 %float_n0_5
        %182 = OpCompositeExtract %float %145 1
        %183 = OpFOrdLessThan %bool %182 %float_0_5
        %184 = OpSelect %float %183 %float_0 %float_n0_5
        %185 = OpCompositeConstruct %v2float %181 %184
        %186 = OpVectorShuffle %v2float %145 %145 0 1
        %187 = OpFAdd %v2float %186 %185
        %188 = OpFMul %v2float %187 %61
        %189 = OpVectorShuffle %v2float %178 %178 0 1
        %190 = OpAccessChain %_ptr_Uniform_v2float %EmitterDynamicUniforms %int_0
        %191 = OpLoad %v2float %190
        %192 = OpFMul %v2float %189 %191
        %193 = OpAccessChain %_ptr_Uniform_v4float %EmitterUniforms %int_6
        %194 = OpLoad %v4float %193
        %195 = OpVectorShuffle %v2float %194 %194 0 1
        %196 = OpCompositeConstruct %v2float %161 %161
        %197 = OpFMul %v2float %195 %196
        %198 = OpExtInst %v2float %1 FMax %197 %63
        %199 = OpVectorShuffle %v2float %194 %194 2 3
        %200 = OpExtInst %v2float %1 FMin %198 %199
        %201 = OpFMul %v2float %188 %192
        %202 = OpFMul %v2float %201 %200
        %203 = OpCompositeConstruct %v2float %147 %147
        %204 = OpFMul %v2float %202 %203
        %205 = OpCompositeExtract %float %145 3
        %206 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_11
        %207 = OpLoad %float %206
        %208 = OpFMul %float %205 %207
        %209 = OpCompositeExtract %float %145 2
        %210 = OpFMul %float %208 %146
        %211 = OpFAdd %float %209 %210
        %212 = OpFMul %float %211 %float_6_28318548
        %213 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_0
        %214 = OpLoad %v4float %213
        %215 = OpVectorShuffle %v3float %214 %214 0 1 2
        %216 = OpVectorShuffle %v3float %137 %108 0 0 0
        %217 = OpFMul %v3float %215 %216
        %218 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_1
        %219 = OpLoad %v4float %218
        %220 = OpVectorShuffle %v3float %219 %219 0 1 2
        %221 = OpVectorShuffle %v3float %137 %108 1 1 1
        %222 = OpFMul %v3float %220 %221
        %223 = OpFAdd %v3float %217 %222
        %224 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_2
        %225 = OpLoad %v4float %224
        %226 = OpVectorShuffle %v3float %225 %225 0 1 2
        %227 = OpVectorShuffle %v3float %137 %108 2 2 2
        %228 = OpFMul %v3float %226 %227
        %229 = OpFAdd %v3float %223 %228
        %230 = OpAccessChain %_ptr_Uniform_v4float %Primitive %int_0 %uint_3
        %231 = OpLoad %v4float %230
        %232 = OpVectorShuffle %v3float %231 %231 0 1 2
        %233 = OpFAdd %v3float %232 %124
        %234 = OpFAdd %v3float %229 %233
        %235 = OpCompositeExtract %float %234 0
        %236 = OpCompositeExtract %float %234 1
        %237 = OpCompositeExtract %float %234 2
        %238 = OpCompositeConstruct %v4float %235 %236 %237 %float_1
        %239 = OpVectorShuffle %v3float %238 %238 0 1 2
        %240 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_10
        %241 = OpLoad %float %240
        %242 = OpCompositeConstruct %v3float %241 %241 %241
        %243 = OpExtInst %v3float %1 FMix %116 %120 %242
        %244 = OpExtInst %v3float %1 FMix %114 %118 %242
        %245 = OpAccessChain %_ptr_Uniform_v4float %EmitterDynamicUniforms %int_3
        %246 = OpLoad %v4float %245
        %247 = OpVectorShuffle %v3float %246 %246 0 1 2
        %248 = OpAccessChain %_ptr_Uniform_float %EmitterDynamicUniforms %int_3 %int_3
        %249 = OpLoad %float %248
        %250 = OpCompositeConstruct %v3float %249 %249 %249
        %251 = OpExtInst %v3float %1 FMix %243 %247 %250
        %252 = OpFNegate %v3float %244
        %253 = OpAccessChain %_ptr_Uniform_v4float %EmitterDynamicUniforms %int_4
        %254 = OpLoad %v4float %253
        %255 = OpVectorShuffle %v3float %254 %254 0 1 2
        %256 = OpAccessChain %_ptr_Uniform_float %EmitterDynamicUniforms %int_4 %int_3
        %257 = OpLoad %float %256
        %258 = OpCompositeConstruct %v3float %257 %257 %257
        %259 = OpExtInst %v3float %1 FMix %252 %255 %258
        %260 = OpFSub %v3float %122 %239
        %261 = OpDot %float %260 %260
        %262 = OpExtInst %float %1 FMax %261 %float_0_00999999978
        %263 = OpExtInst %float %1 Sqrt %262
        %264 = OpCompositeConstruct %v3float %263 %263 %263
        %265 = OpFDiv %v3float %260 %264
        %266 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_9 %int_0
        %267 = OpLoad %float %266
        %268 = OpFOrdGreaterThan %bool %267 %float_0
               OpSelectionMerge %269 DontFlatten
               OpBranchConditional %268 %270 %271
        %270 = OpLabel
        %272 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_9 %int_1
        %273 = OpLoad %float %272
        %274 = OpFMul %float %261 %273
        %275 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_9 %int_2
        %276 = OpLoad %float %275
        %277 = OpFSub %float %274 %276
        %278 = OpExtInst %float %1 FClamp %277 %float_0 %float_1
        %279 = OpExtInst %v3float %1 Cross %265 %82
        %280 = OpDot %float %279 %279
        %281 = OpExtInst %float %1 FMax %280 %float_0_00999999978
        %282 = OpExtInst %float %1 Sqrt %281
        %283 = OpCompositeConstruct %v3float %282 %282 %282
        %284 = OpFDiv %v3float %279 %283
        %285 = OpExtInst %v3float %1 Cross %265 %284
        %286 = OpCompositeConstruct %v3float %278 %278 %278
        %287 = OpExtInst %v3float %1 FMix %251 %284 %286
        %288 = OpExtInst %v3float %1 Normalize %287
        %289 = OpExtInst %v3float %1 FMix %259 %285 %286
        %290 = OpExtInst %v3float %1 Normalize %289
               OpBranch %269
        %271 = OpLabel
        %291 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_8 %int_1
        %292 = OpLoad %float %291
        %293 = OpFOrdGreaterThan %bool %292 %float_0
               OpSelectionMerge %294 Flatten
               OpBranchConditional %293 %295 %296
        %295 = OpLabel
        %297 = OpExtInst %v3float %1 Cross %265 %160
        %298 = OpDot %float %297 %297
        %299 = OpExtInst %float %1 FMax %298 %float_0_00999999978
        %300 = OpExtInst %float %1 Sqrt %299
        %301 = OpCompositeConstruct %v3float %300 %300 %300
        %302 = OpFDiv %v3float %297 %301
        %303 = OpFNegate %v3float %160
               OpBranch %294
        %296 = OpLabel
        %304 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_8 %int_2
        %305 = OpLoad %float %304
        %306 = OpFOrdGreaterThan %bool %305 %float_0
               OpSelectionMerge %307 None
               OpBranchConditional %306 %308 %309
        %308 = OpLabel
        %310 = OpExtInst %v3float %1 Cross %247 %265
        %311 = OpDot %float %310 %310
        %312 = OpExtInst %float %1 FMax %311 %float_0_00999999978
        %313 = OpExtInst %float %1 Sqrt %312
        %314 = OpCompositeConstruct %v3float %313 %313 %313
        %315 = OpFDiv %v3float %310 %314
        %316 = OpFNegate %v3float %315
               OpBranch %307
        %309 = OpLabel
        %317 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_8 %int_3
        %318 = OpLoad %float %317
        %319 = OpFOrdGreaterThan %bool %318 %float_0
               OpSelectionMerge %320 None
               OpBranchConditional %319 %321 %320
        %321 = OpLabel
        %322 = OpExtInst %v3float %1 Cross %265 %82
        %323 = OpDot %float %322 %322
        %324 = OpExtInst %float %1 FMax %323 %float_0_00999999978
        %325 = OpExtInst %float %1 Sqrt %324
        %326 = OpCompositeConstruct %v3float %325 %325 %325
        %327 = OpFDiv %v3float %322 %326
        %328 = OpExtInst %v3float %1 Cross %265 %327
               OpBranch %320
        %320 = OpLabel
        %329 = OpPhi %v3float %251 %309 %327 %321
        %330 = OpPhi %v3float %259 %309 %328 %321
               OpBranch %307
        %307 = OpLabel
        %331 = OpPhi %v3float %247 %308 %329 %320
        %332 = OpPhi %v3float %316 %308 %330 %320
               OpBranch %294
        %294 = OpLabel
        %333 = OpPhi %v3float %302 %295 %331 %307
        %334 = OpPhi %v3float %303 %295 %332 %307
               OpBranch %269
        %269 = OpLabel
        %335 = OpPhi %v3float %288 %270 %333 %294
        %336 = OpPhi %v3float %290 %270 %334 %294
        %337 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_12
        %338 = OpLoad %float %337
        %339 = OpFAdd %float %212 %338
        %340 = OpExtInst %float %1 Sin %339
        %341 = OpExtInst %float %1 Cos %339
        %342 = OpCompositeConstruct %v3float %340 %340 %340
        %343 = OpFMul %v3float %342 %336
        %344 = OpCompositeConstruct %v3float %341 %341 %341
        %345 = OpFMul %v3float %344 %335
        %346 = OpFAdd %v3float %343 %345
        %347 = OpFMul %v3float %344 %336
        %348 = OpFMul %v3float %342 %335
        %349 = OpFSub %v3float %347 %348
        %350 = OpCompositeExtract %float %204 0
        %351 = OpCompositeExtract %float %112 0
        %352 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_15 %int_0
        %353 = OpLoad %float %352
        %354 = OpFAdd %float %351 %353
        %355 = OpFMul %float %350 %354
        %356 = OpCompositeConstruct %v3float %355 %355 %355
        %357 = OpFMul %v3float %356 %346
        %358 = OpCompositeExtract %float %204 1
        %359 = OpCompositeExtract %float %112 1
        %360 = OpAccessChain %_ptr_Uniform_float %EmitterUniforms %int_15 %int_1
        %361 = OpLoad %float %360
        %362 = OpFAdd %float %359 %361
        %363 = OpFMul %float %358 %362
        %364 = OpCompositeConstruct %v3float %363 %363 %363
        %365 = OpFMul %v3float %364 %349
        %366 = OpFAdd %v3float %357 %365
        %367 = OpFAdd %v3float %239 %366
        %368 = OpCompositeExtract %float %367 0
        %369 = OpCompositeExtract %float %367 1
        %370 = OpCompositeExtract %float %367 2
        %371 = OpCompositeConstruct %v4float %368 %369 %370 %float_1
        %372 = OpVectorShuffle %v4float %371 %371 4 5 6 3
        %373 = OpAccessChain %_ptr_Uniform_mat4v4float %MobileShadowDepthPass %int_20
        %374 = OpLoad %mat4v4float %373
        %375 = OpMatrixTimesVector %v4float %374 %372
        %376 = OpAccessChain %_ptr_Uniform_float %MobileShadowDepthPass %int_22
        %377 = OpLoad %float %376
        %378 = OpFOrdGreaterThan %bool %377 %float_0
        %379 = OpCompositeExtract %float %375 2
        %380 = OpFOrdLessThan %bool %379 %float_0
        %381 = OpLogicalAnd %bool %378 %380
               OpSelectionMerge %382 None
               OpBranchConditional %381 %383 %382
        %383 = OpLabel
        %384 = OpCompositeInsert %v4float %float_9_99999997en07 %375 2
        %385 = OpCompositeInsert %v4float %float_1 %384 3
               OpBranch %382
        %382 = OpLabel
        %386 = OpPhi %v4float %375 %269 %385 %383
        %387 = OpAccessChain %_ptr_Uniform_float %MobileShadowDepthPass %int_21 %int_0
        %388 = OpLoad %float %387
        %389 = OpAccessChain %_ptr_Uniform_float %MobileShadowDepthPass %int_21 %int_1
        %390 = OpLoad %float %389
        %391 = OpCompositeExtract %float %386 2
        %392 = OpFMul %float %391 %390
        %393 = OpFAdd %float %392 %388
        %394 = OpCompositeExtract %float %386 3
        %395 = OpFMul %float %393 %394
        %396 = OpCompositeInsert %v4float %395 %386 2
               OpStore %out_var_TEXCOORD6 %float_0
               OpStore %gl_Position %396
               OpReturn
               OpFunctionEnd
