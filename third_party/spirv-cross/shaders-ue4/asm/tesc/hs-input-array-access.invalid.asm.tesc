; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 607
; Schema: 0
               OpCapability Tessellation
               OpCapability SampledBuffer
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint TessellationControl %MainHull "main" %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_VS_To_DS_Position %in_var_VS_To_DS_VertexID %gl_InvocationID %out_var_TEXCOORD10_centroid %out_var_TEXCOORD11_centroid %out_var_VS_To_DS_Position %out_var_VS_To_DS_VertexID %out_var_PN_POSITION %out_var_PN_DisplacementScales %out_var_PN_TessellationMultiplier %out_var_PN_WorldDisplacementMultiplier %out_var_PN_DominantVertex %out_var_PN_DominantVertex1 %out_var_PN_DominantVertex2 %out_var_PN_DominantEdge %out_var_PN_DominantEdge1 %out_var_PN_DominantEdge2 %out_var_PN_DominantEdge3 %out_var_PN_DominantEdge4 %out_var_PN_DominantEdge5 %gl_TessLevelOuter %gl_TessLevelInner %out_var_PN_POSITION9
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
               OpMemberName %FPNTessellationHSToDS 5 "DominantVertex"
               OpMemberName %FPNTessellationHSToDS 6 "DominantEdge"
               OpName %FHitProxyVSToDS "FHitProxyVSToDS"
               OpMemberName %FHitProxyVSToDS 0 "FactoryInterpolants"
               OpMemberName %FHitProxyVSToDS 1 "Position"
               OpMemberName %FHitProxyVSToDS 2 "VertexID"
               OpName %FVertexFactoryInterpolantsVSToDS "FVertexFactoryInterpolantsVSToDS"
               OpMemberName %FVertexFactoryInterpolantsVSToDS 0 "InterpolantsVSToPS"
               OpName %FVertexFactoryInterpolantsVSToPS "FVertexFactoryInterpolantsVSToPS"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 0 "TangentToWorld0"
               OpMemberName %FVertexFactoryInterpolantsVSToPS 1 "TangentToWorld2"
               OpName %FHullShaderConstantDominantVertexData "FHullShaderConstantDominantVertexData"
               OpMemberName %FHullShaderConstantDominantVertexData 0 "UV"
               OpMemberName %FHullShaderConstantDominantVertexData 1 "Normal"
               OpMemberName %FHullShaderConstantDominantVertexData 2 "Tangent"
               OpName %FHullShaderConstantDominantEdgeData "FHullShaderConstantDominantEdgeData"
               OpMemberName %FHullShaderConstantDominantEdgeData 0 "UV0"
               OpMemberName %FHullShaderConstantDominantEdgeData 1 "UV1"
               OpMemberName %FHullShaderConstantDominantEdgeData 2 "Normal0"
               OpMemberName %FHullShaderConstantDominantEdgeData 3 "Normal1"
               OpMemberName %FHullShaderConstantDominantEdgeData 4 "Tangent0"
               OpMemberName %FHullShaderConstantDominantEdgeData 5 "Tangent1"
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
               OpName %in_var_VS_To_DS_Position "in.var.VS_To_DS_Position"
               OpName %in_var_VS_To_DS_VertexID "in.var.VS_To_DS_VertexID"
               OpName %out_var_TEXCOORD10_centroid "out.var.TEXCOORD10_centroid"
               OpName %out_var_TEXCOORD11_centroid "out.var.TEXCOORD11_centroid"
               OpName %out_var_VS_To_DS_Position "out.var.VS_To_DS_Position"
               OpName %out_var_VS_To_DS_VertexID "out.var.VS_To_DS_VertexID"
               OpName %out_var_PN_POSITION "out.var.PN_POSITION"
               OpName %out_var_PN_DisplacementScales "out.var.PN_DisplacementScales"
               OpName %out_var_PN_TessellationMultiplier "out.var.PN_TessellationMultiplier"
               OpName %out_var_PN_WorldDisplacementMultiplier "out.var.PN_WorldDisplacementMultiplier"
               OpName %out_var_PN_DominantVertex "out.var.PN_DominantVertex"
               OpName %out_var_PN_DominantVertex1 "out.var.PN_DominantVertex1"
               OpName %out_var_PN_DominantVertex2 "out.var.PN_DominantVertex2"
               OpName %out_var_PN_DominantEdge "out.var.PN_DominantEdge"
               OpName %out_var_PN_DominantEdge1 "out.var.PN_DominantEdge1"
               OpName %out_var_PN_DominantEdge2 "out.var.PN_DominantEdge2"
               OpName %out_var_PN_DominantEdge3 "out.var.PN_DominantEdge3"
               OpName %out_var_PN_DominantEdge4 "out.var.PN_DominantEdge4"
               OpName %out_var_PN_DominantEdge5 "out.var.PN_DominantEdge5"
               OpName %out_var_PN_POSITION9 "out.var.PN_POSITION9"
               OpName %MainHull "MainHull"
               OpName %param_var_I "param.var.I"
               OpName %temp_var_hullMainRetVal "temp.var.hullMainRetVal"
               OpName %if_merge "if.merge"
               OpDecorateString %in_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %in_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %in_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorateString %in_var_VS_To_DS_VertexID UserSemantic "VS_To_DS_VertexID"
               OpDecorate %gl_InvocationID BuiltIn InvocationId
               OpDecorateString %gl_InvocationID UserSemantic "SV_OutputControlPointID"
               OpDecorateString %out_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %out_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %out_var_VS_To_DS_Position UserSemantic "VS_To_DS_Position"
               OpDecorateString %out_var_VS_To_DS_VertexID UserSemantic "VS_To_DS_VertexID"
               OpDecorateString %out_var_PN_POSITION UserSemantic "PN_POSITION"
               OpDecorateString %out_var_PN_DisplacementScales UserSemantic "PN_DisplacementScales"
               OpDecorateString %out_var_PN_TessellationMultiplier UserSemantic "PN_TessellationMultiplier"
               OpDecorateString %out_var_PN_WorldDisplacementMultiplier UserSemantic "PN_WorldDisplacementMultiplier"
               OpDecorateString %out_var_PN_DominantVertex UserSemantic "PN_DominantVertex"
               OpDecorateString %out_var_PN_DominantVertex1 UserSemantic "PN_DominantVertex"
               OpDecorateString %out_var_PN_DominantVertex2 UserSemantic "PN_DominantVertex"
               OpDecorateString %out_var_PN_DominantEdge UserSemantic "PN_DominantEdge"
               OpDecorateString %out_var_PN_DominantEdge1 UserSemantic "PN_DominantEdge"
               OpDecorateString %out_var_PN_DominantEdge2 UserSemantic "PN_DominantEdge"
               OpDecorateString %out_var_PN_DominantEdge3 UserSemantic "PN_DominantEdge"
               OpDecorateString %out_var_PN_DominantEdge4 UserSemantic "PN_DominantEdge"
               OpDecorateString %out_var_PN_DominantEdge5 UserSemantic "PN_DominantEdge"
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
               OpDecorate %in_var_VS_To_DS_Position Location 2
               OpDecorate %in_var_VS_To_DS_VertexID Location 3
               OpDecorate %out_var_PN_DisplacementScales Location 0
               OpDecorate %out_var_PN_DominantEdge Location 1
               OpDecorate %out_var_PN_DominantEdge1 Location 2
               OpDecorate %out_var_PN_DominantEdge2 Location 3
               OpDecorate %out_var_PN_DominantEdge3 Location 4
               OpDecorate %out_var_PN_DominantEdge4 Location 5
               OpDecorate %out_var_PN_DominantEdge5 Location 6
               OpDecorate %out_var_PN_DominantVertex Location 7
               OpDecorate %out_var_PN_DominantVertex1 Location 8
               OpDecorate %out_var_PN_DominantVertex2 Location 9
               OpDecorate %out_var_PN_POSITION Location 10
               OpDecorate %out_var_PN_POSITION9 Location 13
               OpDecorate %out_var_PN_TessellationMultiplier Location 14
               OpDecorate %out_var_PN_WorldDisplacementMultiplier Location 15
               OpDecorate %out_var_TEXCOORD10_centroid Location 16
               OpDecorate %out_var_TEXCOORD11_centroid Location 17
               OpDecorate %out_var_VS_To_DS_Position Location 18
               OpDecorate %out_var_VS_To_DS_VertexID Location 19
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
         %63 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
  %float_0_5 = OpConstant %float 0.5
      %int_3 = OpConstant %int 3
%float_0_333000004 = OpConstant %float 0.333000004
    %float_1 = OpConstant %float 1
         %68 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
   %float_15 = OpConstant %float 15
         %70 = OpConstantComposite %v4float %float_15 %float_15 %float_15 %float_15
%FVertexFactoryInterpolantsVSToPS = OpTypeStruct %v4float %v4float
%FVertexFactoryInterpolantsVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToPS
%FHitProxyVSToDS = OpTypeStruct %FVertexFactoryInterpolantsVSToDS %v4float %uint
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%FHullShaderConstantDominantVertexData = OpTypeStruct %v2float %v4float %v3float
%FHullShaderConstantDominantEdgeData = OpTypeStruct %v2float %v2float %v4float %v4float %v3float %v3float
%FPNTessellationHSToDS = OpTypeStruct %FHitProxyVSToDS %_arr_v4float_uint_3 %v3float %float %float %FHullShaderConstantDominantVertexData %FHullShaderConstantDominantEdgeData
     %uint_9 = OpConstant %uint 9
      %v3int = OpTypeVector %int 3
         %74 = OpConstantComposite %v3int %int_0 %int_0 %int_0
         %75 = OpConstantComposite %v3int %int_3 %int_3 %int_3
    %float_0 = OpConstant %float 0
         %77 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %78 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
     %int_78 = OpConstant %int 78
     %int_15 = OpConstant %int 15
      %int_7 = OpConstant %int 7
     %int_28 = OpConstant %int 28
         %83 = OpConstantComposite %v3int %int_1 %int_1 %int_1
         %84 = OpConstantComposite %v3int %int_2 %int_2 %int_2
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
%_arr_uint_uint_12 = OpTypeArray %uint %uint_12
%_ptr_Input__arr_uint_uint_12 = OpTypePointer Input %_arr_uint_uint_12
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Output__arr_v4float_uint_3 = OpTypePointer Output %_arr_v4float_uint_3
%_arr_uint_uint_3 = OpTypeArray %uint %uint_3
%_ptr_Output__arr_uint_uint_3 = OpTypePointer Output %_arr_uint_uint_3
%_arr__arr_v4float_uint_3_uint_3 = OpTypeArray %_arr_v4float_uint_3 %uint_3
%_ptr_Output__arr__arr_v4float_uint_3_uint_3 = OpTypePointer Output %_arr__arr_v4float_uint_3_uint_3
%_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
%_ptr_Output__arr_v3float_uint_3 = OpTypePointer Output %_arr_v3float_uint_3
%_ptr_Output__arr_float_uint_3 = OpTypePointer Output %_arr_float_uint_3
%_arr_v2float_uint_3 = OpTypeArray %v2float %uint_3
%_ptr_Output__arr_v2float_uint_3 = OpTypePointer Output %_arr_v2float_uint_3
%_arr_float_uint_4 = OpTypeArray %float %uint_4
%_ptr_Output__arr_float_uint_4 = OpTypePointer Output %_arr_float_uint_4
%_arr_float_uint_2 = OpTypeArray %float %uint_2
%_ptr_Output__arr_float_uint_2 = OpTypePointer Output %_arr_float_uint_2
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
        %109 = OpTypeFunction %void
%_arr_FHitProxyVSToDS_uint_12 = OpTypeArray %FHitProxyVSToDS %uint_12
%_ptr_Function__arr_FHitProxyVSToDS_uint_12 = OpTypePointer Function %_arr_FHitProxyVSToDS_uint_12
%_arr_FPNTessellationHSToDS_uint_3 = OpTypeArray %FPNTessellationHSToDS %uint_3
%_ptr_Function__arr_FPNTessellationHSToDS_uint_3 = OpTypePointer Function %_arr_FPNTessellationHSToDS_uint_3
%_ptr_Workgroup__arr_FPNTessellationHSToDS_uint_3 = OpTypePointer Workgroup %_arr_FPNTessellationHSToDS_uint_3
%_ptr_Output_uint = OpTypePointer Output %uint
%_ptr_Output_v3float = OpTypePointer Output %v3float
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Output_v2float = OpTypePointer Output %v2float
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
%in_var_VS_To_DS_Position = OpVariable %_ptr_Input__arr_v4float_uint_12 Input
%in_var_VS_To_DS_VertexID = OpVariable %_ptr_Input__arr_uint_uint_12 Input
%gl_InvocationID = OpVariable %_ptr_Input_uint Input
%out_var_TEXCOORD10_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_TEXCOORD11_centroid = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_VS_To_DS_Position = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_VS_To_DS_VertexID = OpVariable %_ptr_Output__arr_uint_uint_3 Output
%out_var_PN_POSITION = OpVariable %_ptr_Output__arr__arr_v4float_uint_3_uint_3 Output
%out_var_PN_DisplacementScales = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%out_var_PN_TessellationMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%out_var_PN_WorldDisplacementMultiplier = OpVariable %_ptr_Output__arr_float_uint_3 Output
%out_var_PN_DominantVertex = OpVariable %_ptr_Output__arr_v2float_uint_3 Output
%out_var_PN_DominantVertex1 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_PN_DominantVertex2 = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%out_var_PN_DominantEdge = OpVariable %_ptr_Output__arr_v2float_uint_3 Output
%out_var_PN_DominantEdge1 = OpVariable %_ptr_Output__arr_v2float_uint_3 Output
%out_var_PN_DominantEdge2 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_PN_DominantEdge3 = OpVariable %_ptr_Output__arr_v4float_uint_3 Output
%out_var_PN_DominantEdge4 = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%out_var_PN_DominantEdge5 = OpVariable %_ptr_Output__arr_v3float_uint_3 Output
%gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
%gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output
%out_var_PN_POSITION9 = OpVariable %_ptr_Output_v4float Output
        %130 = OpConstantNull %v2float
%float_0_333333343 = OpConstant %float 0.333333343
        %132 = OpConstantComposite %v4float %float_0_333333343 %float_0_333333343 %float_0_333333343 %float_0_333333343
        %133 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%float_0_166666672 = OpConstant %float 0.166666672
        %135 = OpConstantComposite %v4float %float_0_166666672 %float_0_166666672 %float_0_166666672 %float_0_166666672
        %136 = OpUndef %v4float

; XXX: Original asm used Function here, which is wrong.
; This patches the SPIR-V to be correct.
%temp_var_hullMainRetVal = OpVariable %_ptr_Workgroup__arr_FPNTessellationHSToDS_uint_3 Workgroup

   %MainHull = OpFunction %void None %109
        %137 = OpLabel
%param_var_I = OpVariable %_ptr_Function__arr_FHitProxyVSToDS_uint_12 Function
        %138 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD10_centroid
        %139 = OpLoad %_arr_v4float_uint_12 %in_var_TEXCOORD11_centroid
        %140 = OpCompositeExtract %v4float %138 0
        %141 = OpCompositeExtract %v4float %139 0
        %142 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %140 %141
        %143 = OpCompositeExtract %v4float %138 1
        %144 = OpCompositeExtract %v4float %139 1
        %145 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %143 %144
        %146 = OpCompositeExtract %v4float %138 2
        %147 = OpCompositeExtract %v4float %139 2
        %148 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %146 %147
        %149 = OpCompositeExtract %v4float %138 3
        %150 = OpCompositeExtract %v4float %139 3
        %151 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %149 %150
        %152 = OpCompositeExtract %v4float %138 4
        %153 = OpCompositeExtract %v4float %139 4
        %154 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %152 %153
        %155 = OpCompositeExtract %v4float %138 5
        %156 = OpCompositeExtract %v4float %139 5
        %157 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %155 %156
        %158 = OpCompositeExtract %v4float %138 6
        %159 = OpCompositeExtract %v4float %139 6
        %160 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %158 %159
        %161 = OpCompositeExtract %v4float %138 7
        %162 = OpCompositeExtract %v4float %139 7
        %163 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %161 %162
        %164 = OpCompositeExtract %v4float %138 8
        %165 = OpCompositeExtract %v4float %139 8
        %166 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %164 %165
        %167 = OpCompositeExtract %v4float %138 9
        %168 = OpCompositeExtract %v4float %139 9
        %169 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %167 %168
        %170 = OpCompositeExtract %v4float %138 10
        %171 = OpCompositeExtract %v4float %139 10
        %172 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %170 %171
        %173 = OpCompositeExtract %v4float %138 11
        %174 = OpCompositeExtract %v4float %139 11
        %175 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToPS %173 %174
        %176 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %142
        %177 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %145
        %178 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %148
        %179 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %151
        %180 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %154
        %181 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %157
        %182 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %160
        %183 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %163
        %184 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %166
        %185 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %169
        %186 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %172
        %187 = OpCompositeConstruct %FVertexFactoryInterpolantsVSToDS %175
        %188 = OpLoad %_arr_v4float_uint_12 %in_var_VS_To_DS_Position
        %189 = OpLoad %_arr_uint_uint_12 %in_var_VS_To_DS_VertexID
        %190 = OpCompositeExtract %v4float %188 0
        %191 = OpCompositeExtract %uint %189 0
        %192 = OpCompositeConstruct %FHitProxyVSToDS %176 %190 %191
        %193 = OpCompositeExtract %v4float %188 1
        %194 = OpCompositeExtract %uint %189 1
        %195 = OpCompositeConstruct %FHitProxyVSToDS %177 %193 %194
        %196 = OpCompositeExtract %v4float %188 2
        %197 = OpCompositeExtract %uint %189 2
        %198 = OpCompositeConstruct %FHitProxyVSToDS %178 %196 %197
        %199 = OpCompositeExtract %v4float %188 3
        %200 = OpCompositeExtract %uint %189 3
        %201 = OpCompositeConstruct %FHitProxyVSToDS %179 %199 %200
        %202 = OpCompositeExtract %v4float %188 4
        %203 = OpCompositeExtract %uint %189 4
        %204 = OpCompositeConstruct %FHitProxyVSToDS %180 %202 %203
        %205 = OpCompositeExtract %v4float %188 5
        %206 = OpCompositeExtract %uint %189 5
        %207 = OpCompositeConstruct %FHitProxyVSToDS %181 %205 %206
        %208 = OpCompositeExtract %v4float %188 6
        %209 = OpCompositeExtract %uint %189 6
        %210 = OpCompositeConstruct %FHitProxyVSToDS %182 %208 %209
        %211 = OpCompositeExtract %v4float %188 7
        %212 = OpCompositeExtract %uint %189 7
        %213 = OpCompositeConstruct %FHitProxyVSToDS %183 %211 %212
        %214 = OpCompositeExtract %v4float %188 8
        %215 = OpCompositeExtract %uint %189 8
        %216 = OpCompositeConstruct %FHitProxyVSToDS %184 %214 %215
        %217 = OpCompositeExtract %v4float %188 9
        %218 = OpCompositeExtract %uint %189 9
        %219 = OpCompositeConstruct %FHitProxyVSToDS %185 %217 %218
        %220 = OpCompositeExtract %v4float %188 10
        %221 = OpCompositeExtract %uint %189 10
        %222 = OpCompositeConstruct %FHitProxyVSToDS %186 %220 %221
        %223 = OpCompositeExtract %v4float %188 11
        %224 = OpCompositeExtract %uint %189 11
        %225 = OpCompositeConstruct %FHitProxyVSToDS %187 %223 %224
        %226 = OpCompositeConstruct %_arr_FHitProxyVSToDS_uint_12 %192 %195 %198 %201 %204 %207 %210 %213 %216 %219 %222 %225
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
        %258 = OpULessThan %bool %250 %uint_2
        %259 = OpIAdd %uint %250 %uint_1
        %260 = OpSelect %uint %258 %259 %uint_0
        %261 = OpIMul %uint %uint_2 %250
        %262 = OpIAdd %uint %uint_3 %261
        %263 = OpIAdd %uint %261 %uint_4
        %264 = OpIAdd %uint %uint_9 %227
        %265 = OpAccessChain %_ptr_Function_FHitProxyVSToDS %param_var_I %264
        %266 = OpLoad %FHitProxyVSToDS %265
        %267 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %266 0
        %268 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %267 0
        %269 = OpCompositeExtract %v4float %268 0
        %270 = OpCompositeExtract %v4float %268 1
        %271 = OpVectorShuffle %v3float %269 %269 0 1 2
        %272 = OpCompositeExtract %float %270 0
        %273 = OpCompositeExtract %float %270 1
        %274 = OpCompositeExtract %float %270 2
        %275 = OpCompositeConstruct %v4float %272 %273 %274 %float_0
        %276 = OpAccessChain %_ptr_Function_FHitProxyVSToDS %param_var_I %250
        %277 = OpLoad %FHitProxyVSToDS %276
        %278 = OpCompositeExtract %uint %277 2
        %279 = OpAccessChain %_ptr_Function_FHitProxyVSToDS %param_var_I %260
        %280 = OpLoad %FHitProxyVSToDS %279
        %281 = OpCompositeExtract %uint %280 2
        %282 = OpAccessChain %_ptr_Function_FHitProxyVSToDS %param_var_I %262
        %283 = OpLoad %FHitProxyVSToDS %282
        %284 = OpCompositeExtract %uint %283 2
        %285 = OpAccessChain %_ptr_Function_FHitProxyVSToDS %param_var_I %263
        %286 = OpLoad %FHitProxyVSToDS %285
        %287 = OpCompositeExtract %uint %286 2
        %288 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %277 0
        %289 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %288 0
        %290 = OpCompositeExtract %v4float %289 0
        %291 = OpCompositeExtract %v4float %289 1
        %292 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %280 0
        %293 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %292 0
        %294 = OpCompositeExtract %v4float %293 0
        %295 = OpCompositeExtract %v4float %293 1
        %296 = OpULessThan %bool %284 %278
        %297 = OpIEqual %bool %284 %278
        %298 = OpULessThan %bool %287 %281
        %299 = OpLogicalAnd %bool %297 %298
        %300 = OpLogicalOr %bool %296 %299
               OpSelectionMerge %301 None
               OpBranchConditional %300 %302 %301
        %302 = OpLabel
        %303 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %283 0
        %304 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %303 0
        %305 = OpCompositeExtract %v4float %304 0
        %306 = OpCompositeExtract %v4float %304 1
        %307 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %286 0
        %308 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %307 0
        %309 = OpCompositeExtract %v4float %308 0
        %310 = OpCompositeExtract %v4float %308 1
               OpBranch %301
        %301 = OpLabel
        %311 = OpPhi %v4float %294 %137 %309 %302
        %312 = OpPhi %v4float %295 %137 %310 %302
        %313 = OpPhi %v4float %290 %137 %305 %302
        %314 = OpPhi %v4float %291 %137 %306 %302
        %315 = OpVectorShuffle %v3float %313 %313 0 1 2
        %316 = OpVectorShuffle %v3float %311 %311 0 1 2
        %317 = OpCompositeExtract %float %314 0
        %318 = OpCompositeExtract %float %314 1
        %319 = OpCompositeExtract %float %314 2
        %320 = OpCompositeConstruct %v4float %317 %318 %319 %float_0
        %321 = OpCompositeExtract %float %312 0
        %322 = OpCompositeExtract %float %312 1
        %323 = OpCompositeExtract %float %312 2
        %324 = OpCompositeConstruct %v4float %321 %322 %323 %float_0
        %325 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %250 %int_0
        %326 = OpLoad %FVertexFactoryInterpolantsVSToDS %325
        %327 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %326 0
        %328 = OpCompositeExtract %v4float %327 1
        %329 = OpCompositeExtract %float %328 0
        %330 = OpCompositeExtract %float %328 1
        %331 = OpCompositeExtract %float %328 2
        %332 = OpCompositeConstruct %v4float %329 %330 %331 %float_0
        %333 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %252 %int_0
        %334 = OpLoad %FVertexFactoryInterpolantsVSToDS %333
        %335 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %334 0
        %336 = OpCompositeExtract %v4float %335 1
        %337 = OpCompositeExtract %float %336 0
        %338 = OpCompositeExtract %float %336 1
        %339 = OpCompositeExtract %float %336 2
        %340 = OpCompositeConstruct %v4float %337 %338 %339 %float_0
        %341 = OpAccessChain %_ptr_Function_FVertexFactoryInterpolantsVSToDS %param_var_I %253 %int_0
        %342 = OpLoad %FVertexFactoryInterpolantsVSToDS %341
        %343 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %342 0
        %344 = OpCompositeExtract %v4float %343 1
        %345 = OpCompositeExtract %float %344 0
        %346 = OpCompositeExtract %float %344 1
        %347 = OpCompositeExtract %float %344 2
        %348 = OpCompositeConstruct %v4float %345 %346 %347 %float_0
        %349 = OpLoad %v4float %256
        %350 = OpAccessChain %_ptr_Function_v4float %param_var_I %250 %int_1
        %351 = OpLoad %v4float %350
        %352 = OpFMul %v4float %63 %349
        %353 = OpFAdd %v4float %352 %351
        %354 = OpFSub %v4float %351 %349
        %355 = OpDot %float %354 %243
        %356 = OpCompositeConstruct %v4float %355 %355 %355 %355
        %357 = OpFMul %v4float %356 %243
        %358 = OpFSub %v4float %353 %357
        %359 = OpFMul %v4float %358 %132
        %360 = OpAccessChain %_ptr_Function_v4float %param_var_I %252 %int_1
        %361 = OpLoad %v4float %360
        %362 = OpAccessChain %_ptr_Function_v4float %param_var_I %253 %int_1
        %363 = OpLoad %v4float %362
        %364 = OpFMul %v4float %63 %361
        %365 = OpFAdd %v4float %364 %363
        %366 = OpFSub %v4float %363 %361
        %367 = OpDot %float %366 %340
        %368 = OpCompositeConstruct %v4float %367 %367 %367 %367
        %369 = OpFMul %v4float %368 %340
        %370 = OpFSub %v4float %365 %369
        %371 = OpFMul %v4float %370 %132
        %372 = OpFAdd %v4float %359 %371
        %373 = OpFMul %v4float %372 %133
        %374 = OpLoad %v4float %350
        %375 = OpLoad %v4float %256
        %376 = OpFMul %v4float %63 %374
        %377 = OpFAdd %v4float %376 %375
        %378 = OpFSub %v4float %375 %374
        %379 = OpDot %float %378 %332
        %380 = OpCompositeConstruct %v4float %379 %379 %379 %379
        %381 = OpFMul %v4float %380 %332
        %382 = OpFSub %v4float %377 %381
        %383 = OpFMul %v4float %382 %132
        %384 = OpLoad %v4float %362
        %385 = OpLoad %v4float %360
        %386 = OpFMul %v4float %63 %384
        %387 = OpFAdd %v4float %386 %385
        %388 = OpFSub %v4float %385 %384
        %389 = OpDot %float %388 %348
        %390 = OpCompositeConstruct %v4float %389 %389 %389 %389
        %391 = OpFMul %v4float %390 %348
        %392 = OpFSub %v4float %387 %391
        %393 = OpFMul %v4float %392 %132
        %394 = OpFAdd %v4float %383 %393
        %395 = OpFMul %v4float %394 %133
        %396 = OpCompositeConstruct %FHullShaderConstantDominantEdgeData %130 %130 %320 %324 %315 %316
        %397 = OpCompositeConstruct %FHullShaderConstantDominantVertexData %130 %275 %271
        %398 = OpCompositeConstruct %_arr_v4float_uint_3 %257 %373 %395
        %399 = OpCompositeConstruct %FPNTessellationHSToDS %255 %398 %247 %float_1 %float_1 %397 %396
        %400 = OpCompositeExtract %FVertexFactoryInterpolantsVSToDS %255 0
        %401 = OpCompositeExtract %FVertexFactoryInterpolantsVSToPS %400 0
        %402 = OpCompositeExtract %v4float %401 0
        %403 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD10_centroid %227
               OpStore %403 %402
        %404 = OpCompositeExtract %v4float %401 1
        %405 = OpAccessChain %_ptr_Output_v4float %out_var_TEXCOORD11_centroid %227
               OpStore %405 %404
        %406 = OpCompositeExtract %v4float %255 1
        %407 = OpAccessChain %_ptr_Output_v4float %out_var_VS_To_DS_Position %227
               OpStore %407 %406
        %408 = OpCompositeExtract %uint %255 2
        %409 = OpAccessChain %_ptr_Output_uint %out_var_VS_To_DS_VertexID %227
               OpStore %409 %408
        %410 = OpAccessChain %_ptr_Output__arr_v4float_uint_3 %out_var_PN_POSITION %227
               OpStore %410 %398
        %411 = OpAccessChain %_ptr_Output_v3float %out_var_PN_DisplacementScales %227
               OpStore %411 %247
        %412 = OpAccessChain %_ptr_Output_float %out_var_PN_TessellationMultiplier %227
               OpStore %412 %float_1
        %413 = OpAccessChain %_ptr_Output_float %out_var_PN_WorldDisplacementMultiplier %227
               OpStore %413 %float_1
        %414 = OpAccessChain %_ptr_Output_v2float %out_var_PN_DominantVertex %227
               OpStore %414 %130
        %415 = OpAccessChain %_ptr_Output_v4float %out_var_PN_DominantVertex1 %227
               OpStore %415 %275
        %416 = OpAccessChain %_ptr_Output_v3float %out_var_PN_DominantVertex2 %227
               OpStore %416 %271
        %417 = OpAccessChain %_ptr_Output_v2float %out_var_PN_DominantEdge %227
               OpStore %417 %130
        %418 = OpAccessChain %_ptr_Output_v2float %out_var_PN_DominantEdge1 %227
               OpStore %418 %130
        %419 = OpAccessChain %_ptr_Output_v4float %out_var_PN_DominantEdge2 %227
               OpStore %419 %320
        %420 = OpAccessChain %_ptr_Output_v4float %out_var_PN_DominantEdge3 %227
               OpStore %420 %324
        %421 = OpAccessChain %_ptr_Output_v3float %out_var_PN_DominantEdge4 %227
               OpStore %421 %315
        %422 = OpAccessChain %_ptr_Output_v3float %out_var_PN_DominantEdge5 %227
               OpStore %422 %316
        %423 = OpAccessChain %_ptr_Workgroup_FPNTessellationHSToDS %temp_var_hullMainRetVal %227
               OpStore %423 %399
               OpControlBarrier %uint_2 %uint_4 %uint_0
        %424 = OpIEqual %bool %227 %uint_0
               OpSelectionMerge %if_merge None
               OpBranchConditional %424 %425 %if_merge
        %425 = OpLabel
        %426 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_0
        %427 = OpLoad %mat4v4float %426
        %428 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_7
        %429 = OpLoad %mat4v4float %428
        %430 = OpAccessChain %_ptr_Uniform_v3float %View %int_28
        %431 = OpLoad %v3float %430
        %432 = OpAccessChain %_ptr_Uniform_float %View %int_78
        %433 = OpLoad %float %432
        %434 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_0
        %435 = OpLoad %v4float %434
        %436 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_1
        %437 = OpLoad %v4float %436
        %438 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_0 %int_1 %int_2
        %439 = OpLoad %v4float %438
        %440 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_0
        %441 = OpLoad %v4float %440
        %442 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_1
        %443 = OpLoad %v4float %442
        %444 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_1 %int_1 %int_2
        %445 = OpLoad %v4float %444
        %446 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_0
        %447 = OpLoad %v4float %446
        %448 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_1
        %449 = OpLoad %v4float %448
        %450 = OpAccessChain %_ptr_Workgroup_v4float %temp_var_hullMainRetVal %uint_2 %int_1 %int_2
        %451 = OpLoad %v4float %450
        %452 = OpFAdd %v4float %437 %439
        %453 = OpFAdd %v4float %452 %443
        %454 = OpFAdd %v4float %453 %445
        %455 = OpFAdd %v4float %454 %449
        %456 = OpFAdd %v4float %455 %451
        %457 = OpFMul %v4float %456 %135
        %458 = OpFAdd %v4float %447 %441
        %459 = OpFAdd %v4float %458 %435
        %460 = OpFMul %v4float %459 %132
        %461 = OpFSub %v4float %457 %460
        %462 = OpFMul %v4float %461 %133
        %463 = OpFAdd %v4float %457 %462
        %464 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_1 %int_3
        %465 = OpLoad %float %464
        %466 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_2 %int_3
        %467 = OpLoad %float %466
        %468 = OpFAdd %float %465 %467
        %469 = OpFMul %float %float_0_5 %468
        %470 = OpCompositeInsert %v4float %469 %136 0
        %471 = OpLoad %float %466
        %472 = OpAccessChain %_ptr_Workgroup_float %temp_var_hullMainRetVal %uint_0 %int_3
        %473 = OpLoad %float %472
        %474 = OpFAdd %float %471 %473
        %475 = OpFMul %float %float_0_5 %474
        %476 = OpCompositeInsert %v4float %475 %470 1
        %477 = OpLoad %float %472
        %478 = OpLoad %float %464
        %479 = OpFAdd %float %477 %478
        %480 = OpFMul %float %float_0_5 %479
        %481 = OpCompositeInsert %v4float %480 %476 2
        %482 = OpLoad %float %472
        %483 = OpLoad %float %464
        %484 = OpFAdd %float %482 %483
        %485 = OpLoad %float %466
        %486 = OpFAdd %float %484 %485
        %487 = OpFMul %float %float_0_333000004 %486
        %488 = OpCompositeInsert %v4float %487 %481 3
        %489 = OpVectorShuffle %v3float %435 %435 0 1 2
        %490 = OpVectorShuffle %v3float %441 %441 0 1 2
        %491 = OpVectorShuffle %v3float %447 %447 0 1 2
               OpBranch %492
        %492 = OpLabel
               OpLoopMerge %493 %494 None
               OpBranch %495
        %495 = OpLabel
        %496 = OpMatrixTimesVector %v4float %429 %77
        %497 = OpCompositeExtract %float %435 0
        %498 = OpCompositeExtract %float %435 1
        %499 = OpCompositeExtract %float %435 2
        %500 = OpCompositeConstruct %v4float %497 %498 %499 %float_1
        %501 = OpMatrixTimesVector %v4float %427 %500
        %502 = OpVectorShuffle %v3float %501 %501 0 1 2
        %503 = OpVectorShuffle %v3float %496 %496 0 1 2
        %504 = OpFSub %v3float %502 %503
        %505 = OpCompositeExtract %float %501 3
        %506 = OpCompositeExtract %float %496 3
        %507 = OpFAdd %float %505 %506
        %508 = OpCompositeConstruct %v3float %507 %507 %507
        %509 = OpFOrdLessThan %v3bool %504 %508
        %510 = OpSelect %v3int %509 %83 %74
        %511 = OpFAdd %v3float %502 %503
        %512 = OpFNegate %float %505
        %513 = OpFSub %float %512 %506
        %514 = OpCompositeConstruct %v3float %513 %513 %513
        %515 = OpFOrdGreaterThan %v3bool %511 %514
        %516 = OpSelect %v3int %515 %83 %74
        %517 = OpIMul %v3int %84 %516
        %518 = OpIAdd %v3int %510 %517
        %519 = OpCompositeExtract %float %441 0
        %520 = OpCompositeExtract %float %441 1
        %521 = OpCompositeExtract %float %441 2
        %522 = OpCompositeConstruct %v4float %519 %520 %521 %float_1
        %523 = OpMatrixTimesVector %v4float %427 %522
        %524 = OpVectorShuffle %v3float %523 %523 0 1 2
        %525 = OpFSub %v3float %524 %503
        %526 = OpCompositeExtract %float %523 3
        %527 = OpFAdd %float %526 %506
        %528 = OpCompositeConstruct %v3float %527 %527 %527
        %529 = OpFOrdLessThan %v3bool %525 %528
        %530 = OpSelect %v3int %529 %83 %74
        %531 = OpFAdd %v3float %524 %503
        %532 = OpFNegate %float %526
        %533 = OpFSub %float %532 %506
        %534 = OpCompositeConstruct %v3float %533 %533 %533
        %535 = OpFOrdGreaterThan %v3bool %531 %534
        %536 = OpSelect %v3int %535 %83 %74
        %537 = OpIMul %v3int %84 %536
        %538 = OpIAdd %v3int %530 %537
        %539 = OpBitwiseOr %v3int %518 %538
        %540 = OpCompositeExtract %float %447 0
        %541 = OpCompositeExtract %float %447 1
        %542 = OpCompositeExtract %float %447 2
        %543 = OpCompositeConstruct %v4float %540 %541 %542 %float_1
        %544 = OpMatrixTimesVector %v4float %427 %543
        %545 = OpVectorShuffle %v3float %544 %544 0 1 2
        %546 = OpFSub %v3float %545 %503
        %547 = OpCompositeExtract %float %544 3
        %548 = OpFAdd %float %547 %506
        %549 = OpCompositeConstruct %v3float %548 %548 %548
        %550 = OpFOrdLessThan %v3bool %546 %549
        %551 = OpSelect %v3int %550 %83 %74
        %552 = OpFAdd %v3float %545 %503
        %553 = OpFNegate %float %547
        %554 = OpFSub %float %553 %506
        %555 = OpCompositeConstruct %v3float %554 %554 %554
        %556 = OpFOrdGreaterThan %v3bool %552 %555
        %557 = OpSelect %v3int %556 %83 %74
        %558 = OpIMul %v3int %84 %557
        %559 = OpIAdd %v3int %551 %558
        %560 = OpBitwiseOr %v3int %539 %559
        %561 = OpINotEqual %v3bool %560 %75
        %562 = OpAny %bool %561
               OpSelectionMerge %563 None
               OpBranchConditional %562 %564 %563
        %564 = OpLabel
               OpBranch %493
        %563 = OpLabel
        %565 = OpFSub %v3float %489 %490
        %566 = OpFSub %v3float %490 %491
        %567 = OpFSub %v3float %491 %489
        %568 = OpFAdd %v3float %489 %490
        %569 = OpFMul %v3float %78 %568
        %570 = OpFSub %v3float %569 %431
        %571 = OpFAdd %v3float %490 %491
        %572 = OpFMul %v3float %78 %571
        %573 = OpFSub %v3float %572 %431
        %574 = OpFAdd %v3float %491 %489
        %575 = OpFMul %v3float %78 %574
        %576 = OpFSub %v3float %575 %431
        %577 = OpDot %float %566 %566
        %578 = OpDot %float %573 %573
        %579 = OpFDiv %float %577 %578
        %580 = OpExtInst %float %1 Sqrt %579
        %581 = OpDot %float %567 %567
        %582 = OpDot %float %576 %576
        %583 = OpFDiv %float %581 %582
        %584 = OpExtInst %float %1 Sqrt %583
        %585 = OpDot %float %565 %565
        %586 = OpDot %float %570 %570
        %587 = OpFDiv %float %585 %586
        %588 = OpExtInst %float %1 Sqrt %587
        %589 = OpCompositeConstruct %v4float %580 %584 %588 %float_1
        %590 = OpFAdd %float %580 %584
        %591 = OpFAdd %float %590 %588
        %592 = OpFMul %float %float_0_333000004 %591
        %593 = OpCompositeInsert %v4float %592 %589 3
        %594 = OpCompositeConstruct %v4float %433 %433 %433 %433
        %595 = OpFMul %v4float %594 %593
               OpBranch %493
        %494 = OpLabel
               OpBranch %492
        %493 = OpLabel
        %596 = OpPhi %v4float %77 %564 %595 %563
        %597 = OpFMul %v4float %488 %596
        %598 = OpExtInst %v4float %1 FClamp %597 %68 %70
        %599 = OpCompositeExtract %float %598 0
        %600 = OpCompositeExtract %float %598 1
        %601 = OpCompositeExtract %float %598 2
        %602 = OpCompositeExtract %float %598 3
        %603 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_0
               OpStore %603 %599
        %604 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_1
               OpStore %604 %600
        %605 = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_2
               OpStore %605 %601
        %606 = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0
               OpStore %606 %602
               OpStore %out_var_PN_POSITION9 %463
               OpBranch %if_merge
   %if_merge = OpLabel
               OpReturn
               OpFunctionEnd
