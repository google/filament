; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 271
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPS "main" %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_PRIMITIVE_ID %in_var_TEXCOORD7 %gl_FragCoord %gl_FrontFacing %gl_SampleMask %out_var_SV_Target0 %gl_SampleMask_0
               OpExecutionMode %MainPS OriginUpperLeft
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
               OpName %type_3d_image "type.3d.image"
               OpName %type_sampler "type.sampler"
               OpName %View_SharedBilinearClampedSampler "View_SharedBilinearClampedSampler"
               OpName %type_StructuredBuffer_v4float "type.StructuredBuffer.v4float"
               OpName %View_PrimitiveSceneData "View_PrimitiveSceneData"
               OpName %type_TranslucentBasePass "type.TranslucentBasePass"
               OpMemberName %type_TranslucentBasePass 0 "TranslucentBasePass_Shared_Forward_NumLocalLights"
               OpMemberName %type_TranslucentBasePass 1 "TranslucentBasePass_Shared_Forward_NumReflectionCaptures"
               OpMemberName %type_TranslucentBasePass 2 "TranslucentBasePass_Shared_Forward_HasDirectionalLight"
               OpMemberName %type_TranslucentBasePass 3 "TranslucentBasePass_Shared_Forward_NumGridCells"
               OpMemberName %type_TranslucentBasePass 4 "TranslucentBasePass_Shared_Forward_CulledGridSize"
               OpMemberName %type_TranslucentBasePass 5 "TranslucentBasePass_Shared_Forward_MaxCulledLightsPerCell"
               OpMemberName %type_TranslucentBasePass 6 "TranslucentBasePass_Shared_Forward_LightGridPixelSizeShift"
               OpMemberName %type_TranslucentBasePass 7 "PrePadding_TranslucentBasePass_Shared_Forward_36"
               OpMemberName %type_TranslucentBasePass 8 "PrePadding_TranslucentBasePass_Shared_Forward_40"
               OpMemberName %type_TranslucentBasePass 9 "PrePadding_TranslucentBasePass_Shared_Forward_44"
               OpMemberName %type_TranslucentBasePass 10 "TranslucentBasePass_Shared_Forward_LightGridZParams"
               OpMemberName %type_TranslucentBasePass 11 "PrePadding_TranslucentBasePass_Shared_Forward_60"
               OpMemberName %type_TranslucentBasePass 12 "TranslucentBasePass_Shared_Forward_DirectionalLightDirection"
               OpMemberName %type_TranslucentBasePass 13 "PrePadding_TranslucentBasePass_Shared_Forward_76"
               OpMemberName %type_TranslucentBasePass 14 "TranslucentBasePass_Shared_Forward_DirectionalLightColor"
               OpMemberName %type_TranslucentBasePass 15 "TranslucentBasePass_Shared_Forward_DirectionalLightVolumetricScatteringIntensity"
               OpMemberName %type_TranslucentBasePass 16 "TranslucentBasePass_Shared_Forward_DirectionalLightShadowMapChannelMask"
               OpMemberName %type_TranslucentBasePass 17 "PrePadding_TranslucentBasePass_Shared_Forward_100"
               OpMemberName %type_TranslucentBasePass 18 "TranslucentBasePass_Shared_Forward_DirectionalLightDistanceFadeMAD"
               OpMemberName %type_TranslucentBasePass 19 "TranslucentBasePass_Shared_Forward_NumDirectionalLightCascades"
               OpMemberName %type_TranslucentBasePass 20 "PrePadding_TranslucentBasePass_Shared_Forward_116"
               OpMemberName %type_TranslucentBasePass 21 "PrePadding_TranslucentBasePass_Shared_Forward_120"
               OpMemberName %type_TranslucentBasePass 22 "PrePadding_TranslucentBasePass_Shared_Forward_124"
               OpMemberName %type_TranslucentBasePass 23 "TranslucentBasePass_Shared_Forward_CascadeEndDepths"
               OpMemberName %type_TranslucentBasePass 24 "TranslucentBasePass_Shared_Forward_DirectionalLightWorldToShadowMatrix"
               OpMemberName %type_TranslucentBasePass 25 "TranslucentBasePass_Shared_Forward_DirectionalLightShadowmapMinMax"
               OpMemberName %type_TranslucentBasePass 26 "TranslucentBasePass_Shared_Forward_DirectionalLightShadowmapAtlasBufferSize"
               OpMemberName %type_TranslucentBasePass 27 "TranslucentBasePass_Shared_Forward_DirectionalLightDepthBias"
               OpMemberName %type_TranslucentBasePass 28 "TranslucentBasePass_Shared_Forward_DirectionalLightUseStaticShadowing"
               OpMemberName %type_TranslucentBasePass 29 "PrePadding_TranslucentBasePass_Shared_Forward_488"
               OpMemberName %type_TranslucentBasePass 30 "PrePadding_TranslucentBasePass_Shared_Forward_492"
               OpMemberName %type_TranslucentBasePass 31 "TranslucentBasePass_Shared_Forward_DirectionalLightStaticShadowBufferSize"
               OpMemberName %type_TranslucentBasePass 32 "TranslucentBasePass_Shared_Forward_DirectionalLightWorldToStaticShadow"
               OpMemberName %type_TranslucentBasePass 33 "PrePadding_TranslucentBasePass_Shared_ForwardISR_576"
               OpMemberName %type_TranslucentBasePass 34 "PrePadding_TranslucentBasePass_Shared_ForwardISR_580"
               OpMemberName %type_TranslucentBasePass 35 "PrePadding_TranslucentBasePass_Shared_ForwardISR_584"
               OpMemberName %type_TranslucentBasePass 36 "PrePadding_TranslucentBasePass_Shared_ForwardISR_588"
               OpMemberName %type_TranslucentBasePass 37 "PrePadding_TranslucentBasePass_Shared_ForwardISR_592"
               OpMemberName %type_TranslucentBasePass 38 "PrePadding_TranslucentBasePass_Shared_ForwardISR_596"
               OpMemberName %type_TranslucentBasePass 39 "PrePadding_TranslucentBasePass_Shared_ForwardISR_600"
               OpMemberName %type_TranslucentBasePass 40 "PrePadding_TranslucentBasePass_Shared_ForwardISR_604"
               OpMemberName %type_TranslucentBasePass 41 "PrePadding_TranslucentBasePass_Shared_ForwardISR_608"
               OpMemberName %type_TranslucentBasePass 42 "PrePadding_TranslucentBasePass_Shared_ForwardISR_612"
               OpMemberName %type_TranslucentBasePass 43 "PrePadding_TranslucentBasePass_Shared_ForwardISR_616"
               OpMemberName %type_TranslucentBasePass 44 "PrePadding_TranslucentBasePass_Shared_ForwardISR_620"
               OpMemberName %type_TranslucentBasePass 45 "PrePadding_TranslucentBasePass_Shared_ForwardISR_624"
               OpMemberName %type_TranslucentBasePass 46 "PrePadding_TranslucentBasePass_Shared_ForwardISR_628"
               OpMemberName %type_TranslucentBasePass 47 "PrePadding_TranslucentBasePass_Shared_ForwardISR_632"
               OpMemberName %type_TranslucentBasePass 48 "PrePadding_TranslucentBasePass_Shared_ForwardISR_636"
               OpMemberName %type_TranslucentBasePass 49 "TranslucentBasePass_Shared_ForwardISR_NumLocalLights"
               OpMemberName %type_TranslucentBasePass 50 "TranslucentBasePass_Shared_ForwardISR_NumReflectionCaptures"
               OpMemberName %type_TranslucentBasePass 51 "TranslucentBasePass_Shared_ForwardISR_HasDirectionalLight"
               OpMemberName %type_TranslucentBasePass 52 "TranslucentBasePass_Shared_ForwardISR_NumGridCells"
               OpMemberName %type_TranslucentBasePass 53 "TranslucentBasePass_Shared_ForwardISR_CulledGridSize"
               OpMemberName %type_TranslucentBasePass 54 "TranslucentBasePass_Shared_ForwardISR_MaxCulledLightsPerCell"
               OpMemberName %type_TranslucentBasePass 55 "TranslucentBasePass_Shared_ForwardISR_LightGridPixelSizeShift"
               OpMemberName %type_TranslucentBasePass 56 "PrePadding_TranslucentBasePass_Shared_ForwardISR_676"
               OpMemberName %type_TranslucentBasePass 57 "PrePadding_TranslucentBasePass_Shared_ForwardISR_680"
               OpMemberName %type_TranslucentBasePass 58 "PrePadding_TranslucentBasePass_Shared_ForwardISR_684"
               OpMemberName %type_TranslucentBasePass 59 "TranslucentBasePass_Shared_ForwardISR_LightGridZParams"
               OpMemberName %type_TranslucentBasePass 60 "PrePadding_TranslucentBasePass_Shared_ForwardISR_700"
               OpMemberName %type_TranslucentBasePass 61 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightDirection"
               OpMemberName %type_TranslucentBasePass 62 "PrePadding_TranslucentBasePass_Shared_ForwardISR_716"
               OpMemberName %type_TranslucentBasePass 63 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightColor"
               OpMemberName %type_TranslucentBasePass 64 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightVolumetricScatteringIntensity"
               OpMemberName %type_TranslucentBasePass 65 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightShadowMapChannelMask"
               OpMemberName %type_TranslucentBasePass 66 "PrePadding_TranslucentBasePass_Shared_ForwardISR_740"
               OpMemberName %type_TranslucentBasePass 67 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightDistanceFadeMAD"
               OpMemberName %type_TranslucentBasePass 68 "TranslucentBasePass_Shared_ForwardISR_NumDirectionalLightCascades"
               OpMemberName %type_TranslucentBasePass 69 "PrePadding_TranslucentBasePass_Shared_ForwardISR_756"
               OpMemberName %type_TranslucentBasePass 70 "PrePadding_TranslucentBasePass_Shared_ForwardISR_760"
               OpMemberName %type_TranslucentBasePass 71 "PrePadding_TranslucentBasePass_Shared_ForwardISR_764"
               OpMemberName %type_TranslucentBasePass 72 "TranslucentBasePass_Shared_ForwardISR_CascadeEndDepths"
               OpMemberName %type_TranslucentBasePass 73 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightWorldToShadowMatrix"
               OpMemberName %type_TranslucentBasePass 74 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightShadowmapMinMax"
               OpMemberName %type_TranslucentBasePass 75 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightShadowmapAtlasBufferSize"
               OpMemberName %type_TranslucentBasePass 76 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightDepthBias"
               OpMemberName %type_TranslucentBasePass 77 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightUseStaticShadowing"
               OpMemberName %type_TranslucentBasePass 78 "PrePadding_TranslucentBasePass_Shared_ForwardISR_1128"
               OpMemberName %type_TranslucentBasePass 79 "PrePadding_TranslucentBasePass_Shared_ForwardISR_1132"
               OpMemberName %type_TranslucentBasePass 80 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightStaticShadowBufferSize"
               OpMemberName %type_TranslucentBasePass 81 "TranslucentBasePass_Shared_ForwardISR_DirectionalLightWorldToStaticShadow"
               OpMemberName %type_TranslucentBasePass 82 "PrePadding_TranslucentBasePass_Shared_Reflection_1216"
               OpMemberName %type_TranslucentBasePass 83 "PrePadding_TranslucentBasePass_Shared_Reflection_1220"
               OpMemberName %type_TranslucentBasePass 84 "PrePadding_TranslucentBasePass_Shared_Reflection_1224"
               OpMemberName %type_TranslucentBasePass 85 "PrePadding_TranslucentBasePass_Shared_Reflection_1228"
               OpMemberName %type_TranslucentBasePass 86 "PrePadding_TranslucentBasePass_Shared_Reflection_1232"
               OpMemberName %type_TranslucentBasePass 87 "PrePadding_TranslucentBasePass_Shared_Reflection_1236"
               OpMemberName %type_TranslucentBasePass 88 "PrePadding_TranslucentBasePass_Shared_Reflection_1240"
               OpMemberName %type_TranslucentBasePass 89 "PrePadding_TranslucentBasePass_Shared_Reflection_1244"
               OpMemberName %type_TranslucentBasePass 90 "PrePadding_TranslucentBasePass_Shared_Reflection_1248"
               OpMemberName %type_TranslucentBasePass 91 "PrePadding_TranslucentBasePass_Shared_Reflection_1252"
               OpMemberName %type_TranslucentBasePass 92 "PrePadding_TranslucentBasePass_Shared_Reflection_1256"
               OpMemberName %type_TranslucentBasePass 93 "PrePadding_TranslucentBasePass_Shared_Reflection_1260"
               OpMemberName %type_TranslucentBasePass 94 "PrePadding_TranslucentBasePass_Shared_Reflection_1264"
               OpMemberName %type_TranslucentBasePass 95 "PrePadding_TranslucentBasePass_Shared_Reflection_1268"
               OpMemberName %type_TranslucentBasePass 96 "PrePadding_TranslucentBasePass_Shared_Reflection_1272"
               OpMemberName %type_TranslucentBasePass 97 "PrePadding_TranslucentBasePass_Shared_Reflection_1276"
               OpMemberName %type_TranslucentBasePass 98 "TranslucentBasePass_Shared_Reflection_SkyLightParameters"
               OpMemberName %type_TranslucentBasePass 99 "TranslucentBasePass_Shared_Reflection_SkyLightCubemapBrightness"
               OpMemberName %type_TranslucentBasePass 100 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1300"
               OpMemberName %type_TranslucentBasePass 101 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1304"
               OpMemberName %type_TranslucentBasePass 102 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1308"
               OpMemberName %type_TranslucentBasePass 103 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1312"
               OpMemberName %type_TranslucentBasePass 104 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1316"
               OpMemberName %type_TranslucentBasePass 105 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1320"
               OpMemberName %type_TranslucentBasePass 106 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1324"
               OpMemberName %type_TranslucentBasePass 107 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1328"
               OpMemberName %type_TranslucentBasePass 108 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1332"
               OpMemberName %type_TranslucentBasePass 109 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1336"
               OpMemberName %type_TranslucentBasePass 110 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1340"
               OpMemberName %type_TranslucentBasePass 111 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1344"
               OpMemberName %type_TranslucentBasePass 112 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1348"
               OpMemberName %type_TranslucentBasePass 113 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1352"
               OpMemberName %type_TranslucentBasePass 114 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1356"
               OpMemberName %type_TranslucentBasePass 115 "TranslucentBasePass_Shared_PlanarReflection_ReflectionPlane"
               OpMemberName %type_TranslucentBasePass 116 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionOrigin"
               OpMemberName %type_TranslucentBasePass 117 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionXAxis"
               OpMemberName %type_TranslucentBasePass 118 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionYAxis"
               OpMemberName %type_TranslucentBasePass 119 "TranslucentBasePass_Shared_PlanarReflection_InverseTransposeMirrorMatrix"
               OpMemberName %type_TranslucentBasePass 120 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionParameters"
               OpMemberName %type_TranslucentBasePass 121 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1484"
               OpMemberName %type_TranslucentBasePass 122 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionParameters2"
               OpMemberName %type_TranslucentBasePass 123 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1496"
               OpMemberName %type_TranslucentBasePass 124 "PrePadding_TranslucentBasePass_Shared_PlanarReflection_1500"
               OpMemberName %type_TranslucentBasePass 125 "TranslucentBasePass_Shared_PlanarReflection_ProjectionWithExtraFOV"
               OpMemberName %type_TranslucentBasePass 126 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionScreenScaleBias"
               OpMemberName %type_TranslucentBasePass 127 "TranslucentBasePass_Shared_PlanarReflection_PlanarReflectionScreenBound"
               OpMemberName %type_TranslucentBasePass 128 "TranslucentBasePass_Shared_PlanarReflection_bIsStereo"
               OpMemberName %type_TranslucentBasePass 129 "PrePadding_TranslucentBasePass_Shared_Fog_1676"
               OpMemberName %type_TranslucentBasePass 130 "PrePadding_TranslucentBasePass_Shared_Fog_1680"
               OpMemberName %type_TranslucentBasePass 131 "PrePadding_TranslucentBasePass_Shared_Fog_1684"
               OpMemberName %type_TranslucentBasePass 132 "PrePadding_TranslucentBasePass_Shared_Fog_1688"
               OpMemberName %type_TranslucentBasePass 133 "PrePadding_TranslucentBasePass_Shared_Fog_1692"
               OpMemberName %type_TranslucentBasePass 134 "TranslucentBasePass_Shared_Fog_ExponentialFogParameters"
               OpMemberName %type_TranslucentBasePass 135 "TranslucentBasePass_Shared_Fog_ExponentialFogParameters2"
               OpMemberName %type_TranslucentBasePass 136 "TranslucentBasePass_Shared_Fog_ExponentialFogColorParameter"
               OpMemberName %type_TranslucentBasePass 137 "TranslucentBasePass_Shared_Fog_ExponentialFogParameters3"
               OpMemberName %type_TranslucentBasePass 138 "TranslucentBasePass_Shared_Fog_InscatteringLightDirection"
               OpMemberName %type_TranslucentBasePass 139 "TranslucentBasePass_Shared_Fog_DirectionalInscatteringColor"
               OpMemberName %type_TranslucentBasePass 140 "TranslucentBasePass_Shared_Fog_SinCosInscatteringColorCubemapRotation"
               OpMemberName %type_TranslucentBasePass 141 "PrePadding_TranslucentBasePass_Shared_Fog_1800"
               OpMemberName %type_TranslucentBasePass 142 "PrePadding_TranslucentBasePass_Shared_Fog_1804"
               OpMemberName %type_TranslucentBasePass 143 "TranslucentBasePass_Shared_Fog_FogInscatteringTextureParameters"
               OpMemberName %type_TranslucentBasePass 144 "TranslucentBasePass_Shared_Fog_ApplyVolumetricFog"
               OpMemberName %type_TranslucentBasePass 145 "PrePadding_TranslucentBasePass_1824"
               OpMemberName %type_TranslucentBasePass 146 "PrePadding_TranslucentBasePass_1828"
               OpMemberName %type_TranslucentBasePass 147 "PrePadding_TranslucentBasePass_1832"
               OpMemberName %type_TranslucentBasePass 148 "PrePadding_TranslucentBasePass_1836"
               OpMemberName %type_TranslucentBasePass 149 "PrePadding_TranslucentBasePass_1840"
               OpMemberName %type_TranslucentBasePass 150 "PrePadding_TranslucentBasePass_1844"
               OpMemberName %type_TranslucentBasePass 151 "PrePadding_TranslucentBasePass_1848"
               OpMemberName %type_TranslucentBasePass 152 "PrePadding_TranslucentBasePass_1852"
               OpMemberName %type_TranslucentBasePass 153 "PrePadding_TranslucentBasePass_1856"
               OpMemberName %type_TranslucentBasePass 154 "PrePadding_TranslucentBasePass_1860"
               OpMemberName %type_TranslucentBasePass 155 "PrePadding_TranslucentBasePass_1864"
               OpMemberName %type_TranslucentBasePass 156 "PrePadding_TranslucentBasePass_1868"
               OpMemberName %type_TranslucentBasePass 157 "PrePadding_TranslucentBasePass_1872"
               OpMemberName %type_TranslucentBasePass 158 "PrePadding_TranslucentBasePass_1876"
               OpMemberName %type_TranslucentBasePass 159 "PrePadding_TranslucentBasePass_1880"
               OpMemberName %type_TranslucentBasePass 160 "PrePadding_TranslucentBasePass_1884"
               OpMemberName %type_TranslucentBasePass 161 "PrePadding_TranslucentBasePass_1888"
               OpMemberName %type_TranslucentBasePass 162 "PrePadding_TranslucentBasePass_1892"
               OpMemberName %type_TranslucentBasePass 163 "PrePadding_TranslucentBasePass_1896"
               OpMemberName %type_TranslucentBasePass 164 "PrePadding_TranslucentBasePass_1900"
               OpMemberName %type_TranslucentBasePass 165 "PrePadding_TranslucentBasePass_1904"
               OpMemberName %type_TranslucentBasePass 166 "PrePadding_TranslucentBasePass_1908"
               OpMemberName %type_TranslucentBasePass 167 "PrePadding_TranslucentBasePass_1912"
               OpMemberName %type_TranslucentBasePass 168 "PrePadding_TranslucentBasePass_1916"
               OpMemberName %type_TranslucentBasePass 169 "PrePadding_TranslucentBasePass_1920"
               OpMemberName %type_TranslucentBasePass 170 "PrePadding_TranslucentBasePass_1924"
               OpMemberName %type_TranslucentBasePass 171 "PrePadding_TranslucentBasePass_1928"
               OpMemberName %type_TranslucentBasePass 172 "PrePadding_TranslucentBasePass_1932"
               OpMemberName %type_TranslucentBasePass 173 "PrePadding_TranslucentBasePass_1936"
               OpMemberName %type_TranslucentBasePass 174 "PrePadding_TranslucentBasePass_1940"
               OpMemberName %type_TranslucentBasePass 175 "PrePadding_TranslucentBasePass_1944"
               OpMemberName %type_TranslucentBasePass 176 "PrePadding_TranslucentBasePass_1948"
               OpMemberName %type_TranslucentBasePass 177 "PrePadding_TranslucentBasePass_1952"
               OpMemberName %type_TranslucentBasePass 178 "PrePadding_TranslucentBasePass_1956"
               OpMemberName %type_TranslucentBasePass 179 "PrePadding_TranslucentBasePass_1960"
               OpMemberName %type_TranslucentBasePass 180 "PrePadding_TranslucentBasePass_1964"
               OpMemberName %type_TranslucentBasePass 181 "PrePadding_TranslucentBasePass_1968"
               OpMemberName %type_TranslucentBasePass 182 "PrePadding_TranslucentBasePass_1972"
               OpMemberName %type_TranslucentBasePass 183 "PrePadding_TranslucentBasePass_1976"
               OpMemberName %type_TranslucentBasePass 184 "PrePadding_TranslucentBasePass_1980"
               OpMemberName %type_TranslucentBasePass 185 "PrePadding_TranslucentBasePass_1984"
               OpMemberName %type_TranslucentBasePass 186 "PrePadding_TranslucentBasePass_1988"
               OpMemberName %type_TranslucentBasePass 187 "PrePadding_TranslucentBasePass_1992"
               OpMemberName %type_TranslucentBasePass 188 "PrePadding_TranslucentBasePass_1996"
               OpMemberName %type_TranslucentBasePass 189 "PrePadding_TranslucentBasePass_2000"
               OpMemberName %type_TranslucentBasePass 190 "PrePadding_TranslucentBasePass_2004"
               OpMemberName %type_TranslucentBasePass 191 "PrePadding_TranslucentBasePass_2008"
               OpMemberName %type_TranslucentBasePass 192 "PrePadding_TranslucentBasePass_2012"
               OpMemberName %type_TranslucentBasePass 193 "PrePadding_TranslucentBasePass_2016"
               OpMemberName %type_TranslucentBasePass 194 "PrePadding_TranslucentBasePass_2020"
               OpMemberName %type_TranslucentBasePass 195 "PrePadding_TranslucentBasePass_2024"
               OpMemberName %type_TranslucentBasePass 196 "PrePadding_TranslucentBasePass_2028"
               OpMemberName %type_TranslucentBasePass 197 "PrePadding_TranslucentBasePass_2032"
               OpMemberName %type_TranslucentBasePass 198 "PrePadding_TranslucentBasePass_2036"
               OpMemberName %type_TranslucentBasePass 199 "PrePadding_TranslucentBasePass_2040"
               OpMemberName %type_TranslucentBasePass 200 "PrePadding_TranslucentBasePass_2044"
               OpMemberName %type_TranslucentBasePass 201 "PrePadding_TranslucentBasePass_2048"
               OpMemberName %type_TranslucentBasePass 202 "PrePadding_TranslucentBasePass_2052"
               OpMemberName %type_TranslucentBasePass 203 "PrePadding_TranslucentBasePass_2056"
               OpMemberName %type_TranslucentBasePass 204 "PrePadding_TranslucentBasePass_2060"
               OpMemberName %type_TranslucentBasePass 205 "PrePadding_TranslucentBasePass_2064"
               OpMemberName %type_TranslucentBasePass 206 "PrePadding_TranslucentBasePass_2068"
               OpMemberName %type_TranslucentBasePass 207 "PrePadding_TranslucentBasePass_2072"
               OpMemberName %type_TranslucentBasePass 208 "PrePadding_TranslucentBasePass_2076"
               OpMemberName %type_TranslucentBasePass 209 "PrePadding_TranslucentBasePass_2080"
               OpMemberName %type_TranslucentBasePass 210 "PrePadding_TranslucentBasePass_2084"
               OpMemberName %type_TranslucentBasePass 211 "PrePadding_TranslucentBasePass_2088"
               OpMemberName %type_TranslucentBasePass 212 "PrePadding_TranslucentBasePass_2092"
               OpMemberName %type_TranslucentBasePass 213 "PrePadding_TranslucentBasePass_2096"
               OpMemberName %type_TranslucentBasePass 214 "PrePadding_TranslucentBasePass_2100"
               OpMemberName %type_TranslucentBasePass 215 "PrePadding_TranslucentBasePass_2104"
               OpMemberName %type_TranslucentBasePass 216 "PrePadding_TranslucentBasePass_2108"
               OpMemberName %type_TranslucentBasePass 217 "PrePadding_TranslucentBasePass_2112"
               OpMemberName %type_TranslucentBasePass 218 "PrePadding_TranslucentBasePass_2116"
               OpMemberName %type_TranslucentBasePass 219 "PrePadding_TranslucentBasePass_2120"
               OpMemberName %type_TranslucentBasePass 220 "PrePadding_TranslucentBasePass_2124"
               OpMemberName %type_TranslucentBasePass 221 "PrePadding_TranslucentBasePass_2128"
               OpMemberName %type_TranslucentBasePass 222 "PrePadding_TranslucentBasePass_2132"
               OpMemberName %type_TranslucentBasePass 223 "PrePadding_TranslucentBasePass_2136"
               OpMemberName %type_TranslucentBasePass 224 "PrePadding_TranslucentBasePass_2140"
               OpMemberName %type_TranslucentBasePass 225 "TranslucentBasePass_HZBUvFactorAndInvFactor"
               OpMemberName %type_TranslucentBasePass 226 "TranslucentBasePass_PrevScreenPositionScaleBias"
               OpMemberName %type_TranslucentBasePass 227 "TranslucentBasePass_PrevSceneColorPreExposureInv"
               OpName %TranslucentBasePass "TranslucentBasePass"
               OpName %TranslucentBasePass_Shared_Fog_IntegratedLightScattering "TranslucentBasePass_Shared_Fog_IntegratedLightScattering"
               OpName %type_Material "type.Material"
               OpMemberName %type_Material 0 "Material_VectorExpressions"
               OpMemberName %type_Material 1 "Material_ScalarExpressions"
               OpName %Material "Material"
               OpName %in_var_TEXCOORD10_centroid "in.var.TEXCOORD10_centroid"
               OpName %in_var_TEXCOORD11_centroid "in.var.TEXCOORD11_centroid"
               OpName %in_var_PRIMITIVE_ID "in.var.PRIMITIVE_ID"
               OpName %in_var_TEXCOORD7 "in.var.TEXCOORD7"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPS "MainPS"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorateString %in_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %in_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %in_var_PRIMITIVE_ID UserSemantic "PRIMITIVE_ID"
               OpDecorate %in_var_PRIMITIVE_ID Flat
               OpDecorateString %in_var_TEXCOORD7 UserSemantic "TEXCOORD7"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_Position"
               OpDecorate %gl_FrontFacing BuiltIn FrontFacing
               OpDecorateString %gl_FrontFacing UserSemantic "SV_IsFrontFace"
               OpDecorate %gl_FrontFacing Flat
               OpDecorate %gl_SampleMask BuiltIn SampleMask
               OpDecorateString %gl_SampleMask UserSemantic "SV_Coverage"
               OpDecorate %gl_SampleMask Flat
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %gl_SampleMask_0 BuiltIn SampleMask
               OpDecorateString %gl_SampleMask_0 UserSemantic "SV_Coverage"
               OpDecorate %in_var_TEXCOORD10_centroid Location 0
               OpDecorate %in_var_TEXCOORD11_centroid Location 1
               OpDecorate %in_var_PRIMITIVE_ID Location 2
               OpDecorate %in_var_TEXCOORD7 Location 3
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 1
               OpDecorate %View_SharedBilinearClampedSampler DescriptorSet 0
               OpDecorate %View_SharedBilinearClampedSampler Binding 0
               OpDecorate %View_PrimitiveSceneData DescriptorSet 0
               OpDecorate %View_PrimitiveSceneData Binding 0
               OpDecorate %TranslucentBasePass DescriptorSet 0
               OpDecorate %TranslucentBasePass Binding 2
               OpDecorate %TranslucentBasePass_Shared_Fog_IntegratedLightScattering DescriptorSet 0
               OpDecorate %TranslucentBasePass_Shared_Fog_IntegratedLightScattering Binding 0
               OpDecorate %Material DescriptorSet 0
               OpDecorate %Material Binding 3
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
               OpDecorate %_runtimearr_v4float ArrayStride 16
               OpMemberDecorate %type_StructuredBuffer_v4float 0 Offset 0
               OpMemberDecorate %type_StructuredBuffer_v4float 0 NonWritable
               OpDecorate %type_StructuredBuffer_v4float BufferBlock
               OpDecorate %_arr_mat4v4float_uint_4 ArrayStride 64
               OpDecorate %_arr_mat4v4float_uint_2 ArrayStride 64
               OpMemberDecorate %type_TranslucentBasePass 0 Offset 0
               OpMemberDecorate %type_TranslucentBasePass 1 Offset 4
               OpMemberDecorate %type_TranslucentBasePass 2 Offset 8
               OpMemberDecorate %type_TranslucentBasePass 3 Offset 12
               OpMemberDecorate %type_TranslucentBasePass 4 Offset 16
               OpMemberDecorate %type_TranslucentBasePass 5 Offset 28
               OpMemberDecorate %type_TranslucentBasePass 6 Offset 32
               OpMemberDecorate %type_TranslucentBasePass 7 Offset 36
               OpMemberDecorate %type_TranslucentBasePass 8 Offset 40
               OpMemberDecorate %type_TranslucentBasePass 9 Offset 44
               OpMemberDecorate %type_TranslucentBasePass 10 Offset 48
               OpMemberDecorate %type_TranslucentBasePass 11 Offset 60
               OpMemberDecorate %type_TranslucentBasePass 12 Offset 64
               OpMemberDecorate %type_TranslucentBasePass 13 Offset 76
               OpMemberDecorate %type_TranslucentBasePass 14 Offset 80
               OpMemberDecorate %type_TranslucentBasePass 15 Offset 92
               OpMemberDecorate %type_TranslucentBasePass 16 Offset 96
               OpMemberDecorate %type_TranslucentBasePass 17 Offset 100
               OpMemberDecorate %type_TranslucentBasePass 18 Offset 104
               OpMemberDecorate %type_TranslucentBasePass 19 Offset 112
               OpMemberDecorate %type_TranslucentBasePass 20 Offset 116
               OpMemberDecorate %type_TranslucentBasePass 21 Offset 120
               OpMemberDecorate %type_TranslucentBasePass 22 Offset 124
               OpMemberDecorate %type_TranslucentBasePass 23 Offset 128
               OpMemberDecorate %type_TranslucentBasePass 24 Offset 144
               OpMemberDecorate %type_TranslucentBasePass 24 MatrixStride 16
               OpMemberDecorate %type_TranslucentBasePass 24 ColMajor
               OpMemberDecorate %type_TranslucentBasePass 25 Offset 400
               OpMemberDecorate %type_TranslucentBasePass 26 Offset 464
               OpMemberDecorate %type_TranslucentBasePass 27 Offset 480
               OpMemberDecorate %type_TranslucentBasePass 28 Offset 484
               OpMemberDecorate %type_TranslucentBasePass 29 Offset 488
               OpMemberDecorate %type_TranslucentBasePass 30 Offset 492
               OpMemberDecorate %type_TranslucentBasePass 31 Offset 496
               OpMemberDecorate %type_TranslucentBasePass 32 Offset 512
               OpMemberDecorate %type_TranslucentBasePass 32 MatrixStride 16
               OpMemberDecorate %type_TranslucentBasePass 32 ColMajor
               OpMemberDecorate %type_TranslucentBasePass 33 Offset 576
               OpMemberDecorate %type_TranslucentBasePass 34 Offset 580
               OpMemberDecorate %type_TranslucentBasePass 35 Offset 584
               OpMemberDecorate %type_TranslucentBasePass 36 Offset 588
               OpMemberDecorate %type_TranslucentBasePass 37 Offset 592
               OpMemberDecorate %type_TranslucentBasePass 38 Offset 596
               OpMemberDecorate %type_TranslucentBasePass 39 Offset 600
               OpMemberDecorate %type_TranslucentBasePass 40 Offset 604
               OpMemberDecorate %type_TranslucentBasePass 41 Offset 608
               OpMemberDecorate %type_TranslucentBasePass 42 Offset 612
               OpMemberDecorate %type_TranslucentBasePass 43 Offset 616
               OpMemberDecorate %type_TranslucentBasePass 44 Offset 620
               OpMemberDecorate %type_TranslucentBasePass 45 Offset 624
               OpMemberDecorate %type_TranslucentBasePass 46 Offset 628
               OpMemberDecorate %type_TranslucentBasePass 47 Offset 632
               OpMemberDecorate %type_TranslucentBasePass 48 Offset 636
               OpMemberDecorate %type_TranslucentBasePass 49 Offset 640
               OpMemberDecorate %type_TranslucentBasePass 50 Offset 644
               OpMemberDecorate %type_TranslucentBasePass 51 Offset 648
               OpMemberDecorate %type_TranslucentBasePass 52 Offset 652
               OpMemberDecorate %type_TranslucentBasePass 53 Offset 656
               OpMemberDecorate %type_TranslucentBasePass 54 Offset 668
               OpMemberDecorate %type_TranslucentBasePass 55 Offset 672
               OpMemberDecorate %type_TranslucentBasePass 56 Offset 676
               OpMemberDecorate %type_TranslucentBasePass 57 Offset 680
               OpMemberDecorate %type_TranslucentBasePass 58 Offset 684
               OpMemberDecorate %type_TranslucentBasePass 59 Offset 688
               OpMemberDecorate %type_TranslucentBasePass 60 Offset 700
               OpMemberDecorate %type_TranslucentBasePass 61 Offset 704
               OpMemberDecorate %type_TranslucentBasePass 62 Offset 716
               OpMemberDecorate %type_TranslucentBasePass 63 Offset 720
               OpMemberDecorate %type_TranslucentBasePass 64 Offset 732
               OpMemberDecorate %type_TranslucentBasePass 65 Offset 736
               OpMemberDecorate %type_TranslucentBasePass 66 Offset 740
               OpMemberDecorate %type_TranslucentBasePass 67 Offset 744
               OpMemberDecorate %type_TranslucentBasePass 68 Offset 752
               OpMemberDecorate %type_TranslucentBasePass 69 Offset 756
               OpMemberDecorate %type_TranslucentBasePass 70 Offset 760
               OpMemberDecorate %type_TranslucentBasePass 71 Offset 764
               OpMemberDecorate %type_TranslucentBasePass 72 Offset 768
               OpMemberDecorate %type_TranslucentBasePass 73 Offset 784
               OpMemberDecorate %type_TranslucentBasePass 73 MatrixStride 16
               OpMemberDecorate %type_TranslucentBasePass 73 ColMajor
               OpMemberDecorate %type_TranslucentBasePass 74 Offset 1040
               OpMemberDecorate %type_TranslucentBasePass 75 Offset 1104
               OpMemberDecorate %type_TranslucentBasePass 76 Offset 1120
               OpMemberDecorate %type_TranslucentBasePass 77 Offset 1124
               OpMemberDecorate %type_TranslucentBasePass 78 Offset 1128
               OpMemberDecorate %type_TranslucentBasePass 79 Offset 1132
               OpMemberDecorate %type_TranslucentBasePass 80 Offset 1136
               OpMemberDecorate %type_TranslucentBasePass 81 Offset 1152
               OpMemberDecorate %type_TranslucentBasePass 81 MatrixStride 16
               OpMemberDecorate %type_TranslucentBasePass 81 ColMajor
               OpMemberDecorate %type_TranslucentBasePass 82 Offset 1216
               OpMemberDecorate %type_TranslucentBasePass 83 Offset 1220
               OpMemberDecorate %type_TranslucentBasePass 84 Offset 1224
               OpMemberDecorate %type_TranslucentBasePass 85 Offset 1228
               OpMemberDecorate %type_TranslucentBasePass 86 Offset 1232
               OpMemberDecorate %type_TranslucentBasePass 87 Offset 1236
               OpMemberDecorate %type_TranslucentBasePass 88 Offset 1240
               OpMemberDecorate %type_TranslucentBasePass 89 Offset 1244
               OpMemberDecorate %type_TranslucentBasePass 90 Offset 1248
               OpMemberDecorate %type_TranslucentBasePass 91 Offset 1252
               OpMemberDecorate %type_TranslucentBasePass 92 Offset 1256
               OpMemberDecorate %type_TranslucentBasePass 93 Offset 1260
               OpMemberDecorate %type_TranslucentBasePass 94 Offset 1264
               OpMemberDecorate %type_TranslucentBasePass 95 Offset 1268
               OpMemberDecorate %type_TranslucentBasePass 96 Offset 1272
               OpMemberDecorate %type_TranslucentBasePass 97 Offset 1276
               OpMemberDecorate %type_TranslucentBasePass 98 Offset 1280
               OpMemberDecorate %type_TranslucentBasePass 99 Offset 1296
               OpMemberDecorate %type_TranslucentBasePass 100 Offset 1300
               OpMemberDecorate %type_TranslucentBasePass 101 Offset 1304
               OpMemberDecorate %type_TranslucentBasePass 102 Offset 1308
               OpMemberDecorate %type_TranslucentBasePass 103 Offset 1312
               OpMemberDecorate %type_TranslucentBasePass 104 Offset 1316
               OpMemberDecorate %type_TranslucentBasePass 105 Offset 1320
               OpMemberDecorate %type_TranslucentBasePass 106 Offset 1324
               OpMemberDecorate %type_TranslucentBasePass 107 Offset 1328
               OpMemberDecorate %type_TranslucentBasePass 108 Offset 1332
               OpMemberDecorate %type_TranslucentBasePass 109 Offset 1336
               OpMemberDecorate %type_TranslucentBasePass 110 Offset 1340
               OpMemberDecorate %type_TranslucentBasePass 111 Offset 1344
               OpMemberDecorate %type_TranslucentBasePass 112 Offset 1348
               OpMemberDecorate %type_TranslucentBasePass 113 Offset 1352
               OpMemberDecorate %type_TranslucentBasePass 114 Offset 1356
               OpMemberDecorate %type_TranslucentBasePass 115 Offset 1360
               OpMemberDecorate %type_TranslucentBasePass 116 Offset 1376
               OpMemberDecorate %type_TranslucentBasePass 117 Offset 1392
               OpMemberDecorate %type_TranslucentBasePass 118 Offset 1408
               OpMemberDecorate %type_TranslucentBasePass 119 Offset 1424
               OpMemberDecorate %type_TranslucentBasePass 119 MatrixStride 16
               OpMemberDecorate %type_TranslucentBasePass 119 ColMajor
               OpMemberDecorate %type_TranslucentBasePass 120 Offset 1472
               OpMemberDecorate %type_TranslucentBasePass 121 Offset 1484
               OpMemberDecorate %type_TranslucentBasePass 122 Offset 1488
               OpMemberDecorate %type_TranslucentBasePass 123 Offset 1496
               OpMemberDecorate %type_TranslucentBasePass 124 Offset 1500
               OpMemberDecorate %type_TranslucentBasePass 125 Offset 1504
               OpMemberDecorate %type_TranslucentBasePass 125 MatrixStride 16
               OpMemberDecorate %type_TranslucentBasePass 125 ColMajor
               OpMemberDecorate %type_TranslucentBasePass 126 Offset 1632
               OpMemberDecorate %type_TranslucentBasePass 127 Offset 1664
               OpMemberDecorate %type_TranslucentBasePass 128 Offset 1672
               OpMemberDecorate %type_TranslucentBasePass 129 Offset 1676
               OpMemberDecorate %type_TranslucentBasePass 130 Offset 1680
               OpMemberDecorate %type_TranslucentBasePass 131 Offset 1684
               OpMemberDecorate %type_TranslucentBasePass 132 Offset 1688
               OpMemberDecorate %type_TranslucentBasePass 133 Offset 1692
               OpMemberDecorate %type_TranslucentBasePass 134 Offset 1696
               OpMemberDecorate %type_TranslucentBasePass 135 Offset 1712
               OpMemberDecorate %type_TranslucentBasePass 136 Offset 1728
               OpMemberDecorate %type_TranslucentBasePass 137 Offset 1744
               OpMemberDecorate %type_TranslucentBasePass 138 Offset 1760
               OpMemberDecorate %type_TranslucentBasePass 139 Offset 1776
               OpMemberDecorate %type_TranslucentBasePass 140 Offset 1792
               OpMemberDecorate %type_TranslucentBasePass 141 Offset 1800
               OpMemberDecorate %type_TranslucentBasePass 142 Offset 1804
               OpMemberDecorate %type_TranslucentBasePass 143 Offset 1808
               OpMemberDecorate %type_TranslucentBasePass 144 Offset 1820
               OpMemberDecorate %type_TranslucentBasePass 145 Offset 1824
               OpMemberDecorate %type_TranslucentBasePass 146 Offset 1828
               OpMemberDecorate %type_TranslucentBasePass 147 Offset 1832
               OpMemberDecorate %type_TranslucentBasePass 148 Offset 1836
               OpMemberDecorate %type_TranslucentBasePass 149 Offset 1840
               OpMemberDecorate %type_TranslucentBasePass 150 Offset 1844
               OpMemberDecorate %type_TranslucentBasePass 151 Offset 1848
               OpMemberDecorate %type_TranslucentBasePass 152 Offset 1852
               OpMemberDecorate %type_TranslucentBasePass 153 Offset 1856
               OpMemberDecorate %type_TranslucentBasePass 154 Offset 1860
               OpMemberDecorate %type_TranslucentBasePass 155 Offset 1864
               OpMemberDecorate %type_TranslucentBasePass 156 Offset 1868
               OpMemberDecorate %type_TranslucentBasePass 157 Offset 1872
               OpMemberDecorate %type_TranslucentBasePass 158 Offset 1876
               OpMemberDecorate %type_TranslucentBasePass 159 Offset 1880
               OpMemberDecorate %type_TranslucentBasePass 160 Offset 1884
               OpMemberDecorate %type_TranslucentBasePass 161 Offset 1888
               OpMemberDecorate %type_TranslucentBasePass 162 Offset 1892
               OpMemberDecorate %type_TranslucentBasePass 163 Offset 1896
               OpMemberDecorate %type_TranslucentBasePass 164 Offset 1900
               OpMemberDecorate %type_TranslucentBasePass 165 Offset 1904
               OpMemberDecorate %type_TranslucentBasePass 166 Offset 1908
               OpMemberDecorate %type_TranslucentBasePass 167 Offset 1912
               OpMemberDecorate %type_TranslucentBasePass 168 Offset 1916
               OpMemberDecorate %type_TranslucentBasePass 169 Offset 1920
               OpMemberDecorate %type_TranslucentBasePass 170 Offset 1924
               OpMemberDecorate %type_TranslucentBasePass 171 Offset 1928
               OpMemberDecorate %type_TranslucentBasePass 172 Offset 1932
               OpMemberDecorate %type_TranslucentBasePass 173 Offset 1936
               OpMemberDecorate %type_TranslucentBasePass 174 Offset 1940
               OpMemberDecorate %type_TranslucentBasePass 175 Offset 1944
               OpMemberDecorate %type_TranslucentBasePass 176 Offset 1948
               OpMemberDecorate %type_TranslucentBasePass 177 Offset 1952
               OpMemberDecorate %type_TranslucentBasePass 178 Offset 1956
               OpMemberDecorate %type_TranslucentBasePass 179 Offset 1960
               OpMemberDecorate %type_TranslucentBasePass 180 Offset 1964
               OpMemberDecorate %type_TranslucentBasePass 181 Offset 1968
               OpMemberDecorate %type_TranslucentBasePass 182 Offset 1972
               OpMemberDecorate %type_TranslucentBasePass 183 Offset 1976
               OpMemberDecorate %type_TranslucentBasePass 184 Offset 1980
               OpMemberDecorate %type_TranslucentBasePass 185 Offset 1984
               OpMemberDecorate %type_TranslucentBasePass 186 Offset 1988
               OpMemberDecorate %type_TranslucentBasePass 187 Offset 1992
               OpMemberDecorate %type_TranslucentBasePass 188 Offset 1996
               OpMemberDecorate %type_TranslucentBasePass 189 Offset 2000
               OpMemberDecorate %type_TranslucentBasePass 190 Offset 2004
               OpMemberDecorate %type_TranslucentBasePass 191 Offset 2008
               OpMemberDecorate %type_TranslucentBasePass 192 Offset 2012
               OpMemberDecorate %type_TranslucentBasePass 193 Offset 2016
               OpMemberDecorate %type_TranslucentBasePass 194 Offset 2020
               OpMemberDecorate %type_TranslucentBasePass 195 Offset 2024
               OpMemberDecorate %type_TranslucentBasePass 196 Offset 2028
               OpMemberDecorate %type_TranslucentBasePass 197 Offset 2032
               OpMemberDecorate %type_TranslucentBasePass 198 Offset 2036
               OpMemberDecorate %type_TranslucentBasePass 199 Offset 2040
               OpMemberDecorate %type_TranslucentBasePass 200 Offset 2044
               OpMemberDecorate %type_TranslucentBasePass 201 Offset 2048
               OpMemberDecorate %type_TranslucentBasePass 202 Offset 2052
               OpMemberDecorate %type_TranslucentBasePass 203 Offset 2056
               OpMemberDecorate %type_TranslucentBasePass 204 Offset 2060
               OpMemberDecorate %type_TranslucentBasePass 205 Offset 2064
               OpMemberDecorate %type_TranslucentBasePass 206 Offset 2068
               OpMemberDecorate %type_TranslucentBasePass 207 Offset 2072
               OpMemberDecorate %type_TranslucentBasePass 208 Offset 2076
               OpMemberDecorate %type_TranslucentBasePass 209 Offset 2080
               OpMemberDecorate %type_TranslucentBasePass 210 Offset 2084
               OpMemberDecorate %type_TranslucentBasePass 211 Offset 2088
               OpMemberDecorate %type_TranslucentBasePass 212 Offset 2092
               OpMemberDecorate %type_TranslucentBasePass 213 Offset 2096
               OpMemberDecorate %type_TranslucentBasePass 214 Offset 2100
               OpMemberDecorate %type_TranslucentBasePass 215 Offset 2104
               OpMemberDecorate %type_TranslucentBasePass 216 Offset 2108
               OpMemberDecorate %type_TranslucentBasePass 217 Offset 2112
               OpMemberDecorate %type_TranslucentBasePass 218 Offset 2116
               OpMemberDecorate %type_TranslucentBasePass 219 Offset 2120
               OpMemberDecorate %type_TranslucentBasePass 220 Offset 2124
               OpMemberDecorate %type_TranslucentBasePass 221 Offset 2128
               OpMemberDecorate %type_TranslucentBasePass 222 Offset 2132
               OpMemberDecorate %type_TranslucentBasePass 223 Offset 2136
               OpMemberDecorate %type_TranslucentBasePass 224 Offset 2140
               OpMemberDecorate %type_TranslucentBasePass 225 Offset 2144
               OpMemberDecorate %type_TranslucentBasePass 226 Offset 2160
               OpMemberDecorate %type_TranslucentBasePass 227 Offset 2176
               OpDecorate %type_TranslucentBasePass Block
               OpDecorate %_arr_v4float_uint_1 ArrayStride 16
               OpMemberDecorate %type_Material 0 Offset 0
               OpMemberDecorate %type_Material 1 Offset 32
               OpDecorate %type_Material Block
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
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
      %int_2 = OpConstant %int 2
    %float_0 = OpConstant %float 0
         %48 = OpConstantComposite %v3float %float_0 %float_0 %float_0
     %int_10 = OpConstant %int 10
    %int_144 = OpConstant %int 144
     %int_70 = OpConstant %int 70
    %float_1 = OpConstant %float 1
         %53 = OpConstantComposite %v3float %float_1 %float_1 %float_1
%float_0_577000022 = OpConstant %float 0.577000022
         %55 = OpConstantComposite %v3float %float_0_577000022 %float_0_577000022 %float_0_577000022
         %56 = OpConstantComposite %v3float %float_1 %float_1 %float_0
         %57 = OpConstantComposite %v3float %float_0 %float_1 %float_1
  %float_0_5 = OpConstant %float 0.5
         %59 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %int_60 = OpConstant %int 60
         %61 = OpConstantComposite %v2float %float_0_5 %float_0_5
    %uint_26 = OpConstant %uint 26
     %uint_1 = OpConstant %uint 1
     %uint_5 = OpConstant %uint 5
    %uint_19 = OpConstant %uint 19
 %float_n0_5 = OpConstant %float -0.5
         %67 = OpConstantComposite %v2float %float_0_5 %float_n0_5
         %68 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
 %float_0_25 = OpConstant %float 0.25
     %int_31 = OpConstant %int 31
     %int_66 = OpConstant %int 66
    %int_153 = OpConstant %int 153
    %int_155 = OpConstant %int 155
%mat3v3float = OpTypeMatrix %v3float 3
         %75 = OpConstantComposite %v3float %float_0 %float_0 %float_1
   %float_n1 = OpConstant %float -1
%float_0_200000003 = OpConstant %float 0.200000003
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%type_3d_image = OpTypeImage %float 3D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_3d_image = OpTypePointer UniformConstant %type_3d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%_runtimearr_v4float = OpTypeRuntimeArray %v4float
%type_StructuredBuffer_v4float = OpTypeStruct %_runtimearr_v4float
%_ptr_Uniform_type_StructuredBuffer_v4float = OpTypePointer Uniform %type_StructuredBuffer_v4float
      %v3int = OpTypeVector %int 3
%_arr_mat4v4float_uint_4 = OpTypeArray %mat4v4float %uint_4
%mat3v4float = OpTypeMatrix %v4float 3
%_arr_mat4v4float_uint_2 = OpTypeArray %mat4v4float %uint_2
%type_TranslucentBasePass = OpTypeStruct %uint %uint %uint %uint %v3int %uint %uint %uint %uint %uint %v3float %float %v3float %float %v3float %float %uint %uint %v2float %uint %uint %uint %uint %v4float %_arr_mat4v4float_uint_4 %_arr_v4float_uint_4 %v4float %float %uint %uint %uint %v4float %mat4v4float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %v3int %uint %uint %uint %uint %uint %v3float %float %v3float %float %v3float %float %uint %uint %v2float %uint %uint %uint %uint %v4float %_arr_mat4v4float_uint_4 %_arr_v4float_uint_4 %v4float %float %uint %uint %uint %v4float %mat4v4float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %v4float %v4float %v4float %v4float %mat3v4float %v3float %float %v2float %float %float %_arr_mat4v4float_uint_2 %_arr_v4float_uint_2 %v2float %uint %float %float %float %float %float %v4float %v4float %v4float %v4float %v4float %v4float %v2float %float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %float %v4float %v4float %float
%_ptr_Uniform_type_TranslucentBasePass = OpTypePointer Uniform %type_TranslucentBasePass
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%type_Material = OpTypeStruct %_arr_v4float_uint_2 %_arr_v4float_uint_1
%_ptr_Uniform_type_Material = OpTypePointer Uniform %type_Material
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Input_bool = OpTypePointer Input %bool
%_arr_uint_uint_1 = OpTypeArray %uint %uint_1
%_ptr_Input__arr_uint_uint_1 = OpTypePointer Input %_arr_uint_uint_1
%_ptr_Output_v4float = OpTypePointer Output %v4float
%_ptr_Output__arr_uint_uint_1 = OpTypePointer Output %_arr_uint_uint_1
       %void = OpTypeVoid
         %94 = OpTypeFunction %void
%_ptr_Output_uint = OpTypePointer Output %uint
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
     %v3bool = OpTypeVector %bool 3
%_ptr_Uniform_int = OpTypePointer Uniform %int
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%type_sampled_image = OpTypeSampledImage %type_3d_image
       %View = OpVariable %_ptr_Uniform_type_View Uniform
%View_SharedBilinearClampedSampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%View_PrimitiveSceneData = OpVariable %_ptr_Uniform_type_StructuredBuffer_v4float Uniform
%TranslucentBasePass = OpVariable %_ptr_Uniform_type_TranslucentBasePass Uniform
%TranslucentBasePass_Shared_Fog_IntegratedLightScattering = OpVariable %_ptr_UniformConstant_type_3d_image UniformConstant
   %Material = OpVariable %_ptr_Uniform_type_Material Uniform
%in_var_TEXCOORD10_centroid = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD11_centroid = OpVariable %_ptr_Input_v4float Input
%in_var_PRIMITIVE_ID = OpVariable %_ptr_Input_uint Input
%in_var_TEXCOORD7 = OpVariable %_ptr_Input_v4float Input
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%gl_FrontFacing = OpVariable %_ptr_Input_bool Input
%gl_SampleMask = OpVariable %_ptr_Input__arr_uint_uint_1 Input
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
%gl_SampleMask_0 = OpVariable %_ptr_Output__arr_uint_uint_1 Output
        %102 = OpConstantNull %v4float
 %float_n1_5 = OpConstant %float -1.5
    %float_3 = OpConstant %float 3
        %105 = OpConstantComposite %v3float %float_n1 %float_n1_5 %float_3
%float_12_25 = OpConstant %float 12.25
%float_0_00200000009 = OpConstant %float 0.00200000009
        %108 = OpUndef %float
    %uint_15 = OpConstant %uint 15
     %MainPS = OpFunction %void None %94
        %110 = OpLabel
        %111 = OpLoad %v4float %in_var_TEXCOORD10_centroid
        %112 = OpLoad %v4float %in_var_TEXCOORD11_centroid
        %113 = OpLoad %uint %in_var_PRIMITIVE_ID
        %114 = OpLoad %v4float %in_var_TEXCOORD7
        %115 = OpLoad %v4float %gl_FragCoord
        %116 = OpLoad %_arr_uint_uint_1 %gl_SampleMask
        %117 = OpCompositeExtract %uint %116 0
        %118 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_1
        %119 = OpLoad %mat4v4float %118
        %120 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_10
        %121 = OpLoad %mat4v4float %120
        %122 = OpAccessChain %_ptr_Uniform_v3float %View %int_31
        %123 = OpLoad %v3float %122
        %124 = OpAccessChain %_ptr_Uniform_v4float %View %int_66
        %125 = OpLoad %v4float %124
        %126 = OpVectorShuffle %v3float %111 %111 0 1 2
        %127 = OpVectorShuffle %v3float %112 %112 0 1 2
        %128 = OpExtInst %v3float %1 Cross %127 %126
        %129 = OpCompositeExtract %float %112 3
        %130 = OpCompositeConstruct %v3float %129 %129 %129
        %131 = OpFMul %v3float %128 %130
        %132 = OpCompositeConstruct %mat3v3float %126 %131 %127
        %133 = OpCompositeExtract %float %115 0
        %134 = OpCompositeExtract %float %115 1
        %135 = OpCompositeExtract %float %115 2
        %136 = OpCompositeConstruct %v4float %133 %134 %135 %float_1
        %137 = OpMatrixTimesVector %v4float %121 %136
        %138 = OpVectorShuffle %v3float %137 %137 0 1 2
        %139 = OpCompositeExtract %float %137 3
        %140 = OpCompositeConstruct %v3float %139 %139 %139
        %141 = OpFDiv %v3float %138 %140
        %142 = OpFSub %v3float %141 %123
        %143 = OpVectorShuffle %v3float %125 %125 0 1 2
        %144 = OpCompositeExtract %float %125 3
        %145 = OpCompositeConstruct %v3float %144 %144 %144
        %146 = OpFMul %v3float %75 %145
        %147 = OpFAdd %v3float %146 %143
        %148 = OpExtInst %v3float %1 Normalize %147
        %149 = OpMatrixTimesVector %v3float %132 %148
        %150 = OpExtInst %v3float %1 Normalize %149
        %151 = OpExtInst %float %1 Sqrt %float_12_25
        %152 = OpCompositeConstruct %v3float %151 %151 %151
        %153 = OpFDiv %v3float %105 %152
        %154 = OpDot %float %153 %150
        %155 = OpFAdd %float %float_1 %154
        %156 = OpFMul %float %155 %float_0_5
        %157 = OpFAdd %float %156 %float_0_200000003
        %158 = OpAccessChain %_ptr_Uniform_v4float %Material %int_0 %int_1
        %159 = OpLoad %v4float %158
        %160 = OpVectorShuffle %v3float %159 %159 0 1 2
        %161 = OpCompositeConstruct %v3float %157 %157 %157
        %162 = OpFMul %v3float %160 %161
        %163 = OpAccessChain %_ptr_Uniform_float %TranslucentBasePass %int_144
        %164 = OpLoad %float %163
        %165 = OpFOrdGreaterThan %bool %164 %float_0
               OpSelectionMerge %166 None
               OpBranchConditional %165 %167 %166
        %167 = OpLabel
        %168 = OpCompositeExtract %float %142 0
        %169 = OpCompositeExtract %float %142 1
        %170 = OpCompositeExtract %float %142 2
        %171 = OpCompositeConstruct %v4float %168 %169 %170 %float_1
        %172 = OpMatrixTimesVector %v4float %119 %171
        %173 = OpCompositeExtract %float %172 3
        %174 = OpCompositeConstruct %v2float %173 %173
        %175 = OpVectorShuffle %v2float %172 %172 0 1
        %176 = OpFDiv %v2float %175 %174
        %177 = OpVectorShuffle %v2float %176 %102 0 1
        %178 = OpFMul %v2float %177 %67
        %179 = OpFAdd %v2float %178 %61
        %180 = OpCompositeExtract %float %179 0
        %181 = OpCompositeExtract %float %179 1
        %182 = OpAccessChain %_ptr_Uniform_float %View %int_155 %int_0
        %183 = OpLoad %float %182
        %184 = OpFMul %float %173 %183
        %185 = OpAccessChain %_ptr_Uniform_float %View %int_155 %int_1
        %186 = OpLoad %float %185
        %187 = OpFAdd %float %184 %186
        %188 = OpExtInst %float %1 Log2 %187
        %189 = OpAccessChain %_ptr_Uniform_float %View %int_155 %int_2
        %190 = OpLoad %float %189
        %191 = OpFMul %float %188 %190
        %192 = OpAccessChain %_ptr_Uniform_float %View %int_153 %int_2
        %193 = OpLoad %float %192
        %194 = OpFMul %float %191 %193
        %195 = OpCompositeConstruct %v3float %180 %181 %194
               OpSelectionMerge %196 None
               OpBranchConditional %165 %197 %196
        %197 = OpLabel
        %198 = OpLoad %type_3d_image %TranslucentBasePass_Shared_Fog_IntegratedLightScattering
        %199 = OpLoad %type_sampler %View_SharedBilinearClampedSampler
        %200 = OpSampledImage %type_sampled_image %198 %199
        %201 = OpImageSampleExplicitLod %v4float %200 %195 Lod %float_0
               OpBranch %196
        %196 = OpLabel
        %202 = OpPhi %v4float %68 %167 %201 %197
        %203 = OpVectorShuffle %v3float %202 %202 0 1 2
        %204 = OpVectorShuffle %v3float %114 %114 0 1 2
        %205 = OpCompositeExtract %float %202 3
        %206 = OpCompositeConstruct %v3float %205 %205 %205
        %207 = OpFMul %v3float %204 %206
        %208 = OpFAdd %v3float %203 %207
        %209 = OpCompositeExtract %float %208 0
        %210 = OpCompositeExtract %float %208 1
        %211 = OpCompositeExtract %float %208 2
        %212 = OpCompositeExtract %float %114 3
        %213 = OpFMul %float %205 %212
        %214 = OpCompositeConstruct %v4float %209 %210 %211 %213
               OpBranch %166
        %166 = OpLabel
        %215 = OpPhi %v4float %114 %110 %214 %196
        %216 = OpExtInst %v3float %1 FMax %162 %48
        %217 = OpAccessChain %_ptr_Uniform_float %View %int_70
        %218 = OpLoad %float %217
        %219 = OpFOrdGreaterThan %bool %218 %float_0
               OpSelectionMerge %220 DontFlatten
               OpBranchConditional %219 %221 %220
        %221 = OpLabel
        %222 = OpIMul %uint %113 %uint_26
        %223 = OpIAdd %uint %222 %uint_5
        %224 = OpAccessChain %_ptr_Uniform_v4float %View_PrimitiveSceneData %int_0 %223
        %225 = OpLoad %v4float %224
        %226 = OpVectorShuffle %v3float %225 %225 0 1 2
        %227 = OpFSub %v3float %142 %226
        %228 = OpExtInst %v3float %1 FAbs %227
        %229 = OpIAdd %uint %222 %uint_19
        %230 = OpAccessChain %_ptr_Uniform_v4float %View_PrimitiveSceneData %int_0 %229
        %231 = OpLoad %v4float %230
        %232 = OpVectorShuffle %v3float %231 %231 0 1 2
        %233 = OpFAdd %v3float %232 %53
        %234 = OpFOrdGreaterThan %v3bool %228 %233
        %235 = OpAny %bool %234
               OpSelectionMerge %236 None
               OpBranchConditional %235 %237 %236
        %237 = OpLabel
        %238 = OpDot %float %142 %55
        %239 = OpFMul %float %238 %float_0_00200000009
        %240 = OpExtInst %float %1 Fract %239
        %241 = OpCompositeConstruct %v3float %240 %240 %240
        %242 = OpFOrdGreaterThan %v3bool %241 %59
        %243 = OpSelect %v3float %242 %53 %48
        %244 = OpExtInst %v3float %1 FMix %56 %57 %243
               OpBranch %236
        %236 = OpLabel
        %245 = OpPhi %v3float %216 %221 %244 %237
               OpBranch %220
        %220 = OpLabel
        %246 = OpPhi %v3float %216 %166 %245 %236
        %247 = OpCompositeExtract %float %215 3
        %248 = OpCompositeConstruct %v3float %247 %247 %247
        %249 = OpFMul %v3float %246 %248
        %250 = OpVectorShuffle %v3float %215 %215 0 1 2
        %251 = OpFAdd %v3float %249 %250
        %252 = OpCompositeExtract %float %251 0
        %253 = OpCompositeExtract %float %251 1
        %254 = OpCompositeExtract %float %251 2
        %255 = OpCompositeConstruct %v4float %252 %253 %254 %108
        %256 = OpCompositeInsert %v4float %float_1 %255 3
        %257 = OpAccessChain %_ptr_Uniform_int %View %int_60
        %258 = OpLoad %int %257
        %259 = OpSGreaterThan %bool %258 %int_1
               OpSelectionMerge %260 None
               OpBranchConditional %259 %261 %262
        %262 = OpLabel
               OpBranch %260
        %261 = OpLabel
        %263 = OpConvertSToF %float %258
        %264 = OpFMul %float %263 %float_0_25
        %265 = OpCompositeConstruct %v4float %264 %264 %264 %264
        %266 = OpFMul %v4float %256 %265
        %267 = OpBitwiseAnd %uint %117 %uint_15
               OpBranch %260
        %260 = OpLabel
        %268 = OpPhi %v4float %266 %261 %256 %262
        %269 = OpPhi %uint %267 %261 %117 %262
               OpStore %out_var_SV_Target0 %268
        %270 = OpAccessChain %_ptr_Output_uint %gl_SampleMask_0 %uint_0
               OpStore %270 %269
               OpReturn
               OpFunctionEnd
