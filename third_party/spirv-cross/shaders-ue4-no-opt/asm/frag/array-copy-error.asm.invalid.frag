; SPIR-V
; Version: 1.0
; Generator: Google spiregg; 0
; Bound: 353
; Schema: 0
               OpCapability Shader
               OpExtension "SPV_GOOGLE_hlsl_functionality1"
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %MainPixelShader "main" %gl_FragCoord %in_var_TEXCOORD6 %in_var_TEXCOORD7 %in_var_TEXCOORD10_centroid %in_var_TEXCOORD11_centroid %in_var_TEXCOORD0 %in_var_PRIMITIVE_ID %gl_FrontFacing %gl_FragDepth %out_var_SV_Target0
               OpExecutionMode %MainPixelShader OriginUpperLeft
               OpExecutionMode %MainPixelShader DepthReplacing
               OpExecutionMode %MainPixelShader DepthLess
               OpSource HLSL 600
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
               OpName %type_PrimitiveDither "type.PrimitiveDither"
               OpMemberName %type_PrimitiveDither 0 "PrimitiveDither_LODFactor"
               OpName %PrimitiveDither "PrimitiveDither"
               OpName %type_PrimitiveFade "type.PrimitiveFade"
               OpMemberName %type_PrimitiveFade 0 "PrimitiveFade_FadeTimeScaleBias"
               OpName %PrimitiveFade "PrimitiveFade"
               OpName %type_Material "type.Material"
               OpMemberName %type_Material 0 "Material_VectorExpressions"
               OpMemberName %type_Material 1 "Material_ScalarExpressions"
               OpName %Material "Material"
               OpName %type_2d_image "type.2d.image"
               OpName %Material_Texture2D_0 "Material_Texture2D_0"
               OpName %type_sampler "type.sampler"
               OpName %Material_Texture2D_0Sampler "Material_Texture2D_0Sampler"
               OpName %Material_Texture2D_3 "Material_Texture2D_3"
               OpName %Material_Texture2D_3Sampler "Material_Texture2D_3Sampler"
               OpName %in_var_TEXCOORD6 "in.var.TEXCOORD6"
               OpName %in_var_TEXCOORD7 "in.var.TEXCOORD7"
               OpName %in_var_TEXCOORD10_centroid "in.var.TEXCOORD10_centroid"
               OpName %in_var_TEXCOORD11_centroid "in.var.TEXCOORD11_centroid"
               OpName %in_var_TEXCOORD0 "in.var.TEXCOORD0"
               OpName %in_var_PRIMITIVE_ID "in.var.PRIMITIVE_ID"
               OpName %out_var_SV_Target0 "out.var.SV_Target0"
               OpName %MainPixelShader "MainPixelShader"
               OpName %type_sampled_image "type.sampled.image"
               OpDecorate %gl_FragCoord BuiltIn FragCoord
               OpDecorateString %gl_FragCoord UserSemantic "SV_Position"
               OpDecorateString %in_var_TEXCOORD6 UserSemantic "TEXCOORD6"
               OpDecorateString %in_var_TEXCOORD7 UserSemantic "TEXCOORD7"
               OpDecorateString %in_var_TEXCOORD10_centroid UserSemantic "TEXCOORD10_centroid"
               OpDecorateString %in_var_TEXCOORD11_centroid UserSemantic "TEXCOORD11_centroid"
               OpDecorateString %in_var_TEXCOORD0 UserSemantic "TEXCOORD0"
               OpDecorateString %in_var_PRIMITIVE_ID UserSemantic "PRIMITIVE_ID"
               OpDecorate %in_var_PRIMITIVE_ID Flat
               OpDecorate %gl_FrontFacing BuiltIn FrontFacing
               OpDecorateString %gl_FrontFacing UserSemantic "SV_IsFrontFace"
               OpDecorate %gl_FrontFacing Flat
               OpDecorate %gl_FragDepth BuiltIn FragDepth
               OpDecorateString %gl_FragDepth UserSemantic "SV_DepthLessEqual"
               OpDecorateString %out_var_SV_Target0 UserSemantic "SV_Target0"
               OpDecorate %in_var_TEXCOORD6 Location 0
               OpDecorate %in_var_TEXCOORD7 Location 1
               OpDecorate %in_var_TEXCOORD10_centroid Location 2
               OpDecorate %in_var_TEXCOORD11_centroid Location 3
               OpDecorate %in_var_TEXCOORD0 Location 4
               OpDecorate %in_var_PRIMITIVE_ID Location 5
               OpDecorate %out_var_SV_Target0 Location 0
               OpDecorate %View DescriptorSet 0
               OpDecorate %View Binding 0
               OpDecorate %PrimitiveDither DescriptorSet 0
               OpDecorate %PrimitiveDither Binding 1
               OpDecorate %PrimitiveFade DescriptorSet 0
               OpDecorate %PrimitiveFade Binding 2
               OpDecorate %Material DescriptorSet 0
               OpDecorate %Material Binding 3
               OpDecorate %Material_Texture2D_0 DescriptorSet 0
               OpDecorate %Material_Texture2D_0 Binding 0
               OpDecorate %Material_Texture2D_0Sampler DescriptorSet 0
               OpDecorate %Material_Texture2D_0Sampler Binding 0
               OpDecorate %Material_Texture2D_3 DescriptorSet 0
               OpDecorate %Material_Texture2D_3 Binding 1
               OpDecorate %Material_Texture2D_3Sampler DescriptorSet 0
               OpDecorate %Material_Texture2D_3Sampler Binding 1
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
               OpMemberDecorate %type_PrimitiveDither 0 Offset 0
               OpDecorate %type_PrimitiveDither Block
               OpMemberDecorate %type_PrimitiveFade 0 Offset 0
               OpDecorate %type_PrimitiveFade Block
               OpDecorate %_arr_v4float_uint_9 ArrayStride 16
               OpDecorate %_arr_v4float_uint_3 ArrayStride 16
               OpMemberDecorate %type_Material 0 Offset 0
               OpMemberDecorate %type_Material 1 Offset 144
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
      %v2int = OpTypeVector %int 2
%float_0_00100000005 = OpConstant %float 0.00100000005
      %int_2 = OpConstant %int 2
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %float_0 = OpConstant %float 0
         %49 = OpConstantComposite %v2float %float_0 %float_0
    %float_1 = OpConstant %float 1
      %int_4 = OpConstant %int 4
     %int_11 = OpConstant %int 11
%float_0_249500006 = OpConstant %float 0.249500006
         %54 = OpConstantComposite %v2float %float_0_249500006 %float_0_249500006
%float_0_499992371 = OpConstant %float 0.499992371
         %56 = OpConstantComposite %v2float %float_0_499992371 %float_0_499992371
     %int_32 = OpConstant %int 32
     %int_53 = OpConstant %int 53
     %int_57 = OpConstant %int 57
     %int_80 = OpConstant %int 80
     %int_82 = OpConstant %int 82
     %int_98 = OpConstant %int 98
     %uint_1 = OpConstant %uint 1
%mat3v3float = OpTypeMatrix %v3float 3
    %float_2 = OpConstant %float 2
   %float_n1 = OpConstant %float -1
         %67 = OpConstantComposite %v2float %float_n1 %float_n1
       %bool = OpTypeBool
 %float_n0_5 = OpConstant %float -0.5
         %70 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%float_0_333299994 = OpConstant %float 0.333299994
     %uint_5 = OpConstant %uint 5
%float_347_834503 = OpConstant %float 347.834503
%float_3343_28369 = OpConstant %float 3343.28369
         %75 = OpConstantComposite %v2float %float_347_834503 %float_3343_28369
 %float_1000 = OpConstant %float 1000
%_arr_v4float_uint_2 = OpTypeArray %v4float %uint_2
%_arr_v4float_uint_7 = OpTypeArray %v4float %uint_7
%_arr_v4float_uint_4 = OpTypeArray %v4float %uint_4
  %type_View = OpTypeStruct %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %v3float %float %v3float %float %v4float %v4float %v3float %float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %mat4v4float %v3float %float %v3float %float %v3float %float %mat4v4float %mat4v4float %mat4v4float %v4float %v4float %v2float %v2float %v4float %v4float %v4float %v4float %int %float %float %float %v4float %v4float %v4float %v2float %float %float %float %float %float %float %v3float %float %float %float %float %float %float %float %float %uint %uint %uint %uint %float %float %float %float %float %v4float %v3float %float %_arr_v4float_uint_2 %_arr_v4float_uint_2 %v4float %v4float %float %float %float %float %float %float %float %float %float %float %float %float %v3float %float %v3float %float %float %float %float %float %float %float %float %float %float %float %float %float %v4float %uint %uint %uint %uint %v4float %v3float %float %v4float %float %float %float %float %v4float %_arr_v4float_uint_7 %float %float %float %float %uint %float %float %float %v3float %int %_arr_v4float_uint_4 %_arr_v4float_uint_4 %float %float %float %float %v2int %float %float %v3float %float %v3float %float %v2float %float %float %v3float %float %v3float %float %v3float %float %v3float %float %float %float
%_ptr_Uniform_type_View = OpTypePointer Uniform %type_View
%type_PrimitiveDither = OpTypeStruct %float
%_ptr_Uniform_type_PrimitiveDither = OpTypePointer Uniform %type_PrimitiveDither
%type_PrimitiveFade = OpTypeStruct %v2float
%_ptr_Uniform_type_PrimitiveFade = OpTypePointer Uniform %type_PrimitiveFade
     %uint_9 = OpConstant %uint 9
%_arr_v4float_uint_9 = OpTypeArray %v4float %uint_9
     %uint_3 = OpConstant %uint 3
%_arr_v4float_uint_3 = OpTypeArray %v4float %uint_3
%type_Material = OpTypeStruct %_arr_v4float_uint_9 %_arr_v4float_uint_3
%_ptr_Uniform_type_Material = OpTypePointer Uniform %type_Material
%type_2d_image = OpTypeImage %float 2D 2 0 0 1 Unknown
%_ptr_UniformConstant_type_2d_image = OpTypePointer UniformConstant %type_2d_image
%type_sampler = OpTypeSampler
%_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler
%_ptr_Input_v4float = OpTypePointer Input %v4float
%_arr_v4float_uint_1 = OpTypeArray %v4float %uint_1
%_ptr_Input__arr_v4float_uint_1 = OpTypePointer Input %_arr_v4float_uint_1
%_ptr_Input_uint = OpTypePointer Input %uint
%_ptr_Input_bool = OpTypePointer Input %bool
%_ptr_Output_float = OpTypePointer Output %float
%_ptr_Output_v4float = OpTypePointer Output %v4float
       %void = OpTypeVoid
         %93 = OpTypeFunction %void
%_ptr_Uniform_mat4v4float = OpTypePointer Uniform %mat4v4float
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_float = OpTypePointer Uniform %float
%type_sampled_image = OpTypeSampledImage %type_2d_image
       %View = OpVariable %_ptr_Uniform_type_View Uniform
%PrimitiveDither = OpVariable %_ptr_Uniform_type_PrimitiveDither Uniform
%PrimitiveFade = OpVariable %_ptr_Uniform_type_PrimitiveFade Uniform
   %Material = OpVariable %_ptr_Uniform_type_Material Uniform
%Material_Texture2D_0 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%Material_Texture2D_0Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%Material_Texture2D_3 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
%Material_Texture2D_3Sampler = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
%gl_FragCoord = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD6 = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD7 = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD10_centroid = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD11_centroid = OpVariable %_ptr_Input_v4float Input
%in_var_TEXCOORD0 = OpVariable %_ptr_Input__arr_v4float_uint_1 Input
%in_var_PRIMITIVE_ID = OpVariable %_ptr_Input_uint Input
%gl_FrontFacing = OpVariable %_ptr_Input_bool Input
%gl_FragDepth = OpVariable %_ptr_Output_float Output
%out_var_SV_Target0 = OpVariable %_ptr_Output_v4float Output
         %98 = OpUndef %float
         %99 = OpConstantNull %v2float
%float_0_015625 = OpConstant %float 0.015625
        %101 = OpConstantComposite %v2float %float_0_015625 %float_0_015625
%float_0_166666672 = OpConstant %float 0.166666672
        %103 = OpUndef %float
        %104 = OpConstantNull %v3float
%MainPixelShader = OpFunction %void None %93
        %105 = OpLabel
        %106 = OpLoad %v4float %gl_FragCoord
        %107 = OpLoad %v4float %in_var_TEXCOORD6
        %108 = OpLoad %v4float %in_var_TEXCOORD7
        %109 = OpLoad %v4float %in_var_TEXCOORD10_centroid
        %110 = OpLoad %v4float %in_var_TEXCOORD11_centroid
        %111 = OpLoad %_arr_v4float_uint_1 %in_var_TEXCOORD0
        %112 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_4
        %113 = OpLoad %mat4v4float %112
        %114 = OpAccessChain %_ptr_Uniform_mat4v4float %View %int_11
        %115 = OpLoad %mat4v4float %114
        %116 = OpAccessChain %_ptr_Uniform_v3float %View %int_32
        %117 = OpLoad %v3float %116
        %118 = OpAccessChain %_ptr_Uniform_v4float %View %int_53
        %119 = OpLoad %v4float %118
        %120 = OpAccessChain %_ptr_Uniform_v4float %View %int_57
        %121 = OpLoad %v4float %120
        %122 = OpAccessChain %_ptr_Uniform_float %View %int_80
        %123 = OpLoad %float %122
        %124 = OpCompositeExtract %v4float %111 0
        %125 = OpVectorShuffle %v2float %99 %124 2 3
        %126 = OpVectorShuffle %v3float %109 %109 0 1 2
        %127 = OpVectorShuffle %v3float %110 %110 0 1 2
        %128 = OpExtInst %v3float %1 Cross %127 %126
        %129 = OpCompositeExtract %float %110 3
        %130 = OpCompositeConstruct %v3float %129 %129 %129
        %131 = OpFMul %v3float %128 %130
        %132 = OpCompositeConstruct %mat3v3float %126 %131 %127
        %133 = OpVectorShuffle %v2float %106 %106 0 1
        %134 = OpVectorShuffle %v2float %121 %121 0 1
        %135 = OpFSub %v2float %133 %134
        %136 = OpCompositeExtract %float %106 2
        %137 = OpCompositeConstruct %v4float %103 %103 %136 %float_1
        %138 = OpCompositeExtract %float %106 3
        %139 = OpCompositeConstruct %v4float %138 %138 %138 %138
        %140 = OpFMul %v4float %137 %139
        %141 = OpCompositeExtract %float %106 0
        %142 = OpCompositeExtract %float %106 1
        %143 = OpCompositeConstruct %v4float %141 %142 %136 %float_1
        %144 = OpMatrixTimesVector %v4float %115 %143
        %145 = OpVectorShuffle %v3float %144 %144 0 1 2
        %146 = OpCompositeExtract %float %144 3
        %147 = OpCompositeConstruct %v3float %146 %146 %146
        %148 = OpFDiv %v3float %145 %147
        %149 = OpFSub %v3float %148 %117
        %150 = OpFNegate %v3float %148
        %151 = OpExtInst %v3float %1 Normalize %150
        %152 = OpVectorTimesMatrix %v3float %151 %132
        %153 = OpVectorShuffle %v2float %152 %152 0 1
        %154 = OpFMul %v2float %153 %67
        %155 = OpCompositeExtract %float %152 2
        %156 = OpCompositeConstruct %v2float %155 %155
        %157 = OpFDiv %v2float %154 %156
        %158 = OpAccessChain %_ptr_Uniform_float %Material %int_1 %int_0 %int_0
        %159 = OpLoad %float %158
        %160 = OpCompositeConstruct %v2float %159 %159
        %161 = OpFMul %v2float %160 %157
        %162 = OpDot %float %151 %127
        %163 = OpExtInst %float %1 FAbs %162
        %164 = OpExtInst %float %1 FMax %163 %float_0
        %165 = OpExtInst %float %1 FMin %164 %float_1
        %166 = OpAccessChain %_ptr_Uniform_float %Material %int_1 %int_0 %int_1
        %167 = OpLoad %float %166
        %168 = OpAccessChain %_ptr_Uniform_float %Material %int_1 %int_0 %int_2
        %169 = OpLoad %float %168
        %170 = OpExtInst %float %1 FMix %167 %169 %165
        %171 = OpExtInst %float %1 Floor %170
        %172 = OpFDiv %float %float_1 %170
        %173 = OpCompositeConstruct %v2float %172 %172
        %174 = OpFMul %v2float %161 %173
        %175 = OpDPdx %v2float %125
        %176 = OpDPdy %v2float %125
        %177 = OpLoad %type_2d_image %Material_Texture2D_0
        %178 = OpLoad %type_sampler %Material_Texture2D_0Sampler
               OpBranch %179
        %179 = OpLabel
        %180 = OpPhi %float %float_1 %105 %181 %182
        %183 = OpPhi %v2float %49 %105 %184 %182
        %185 = OpPhi %int %int_0 %105 %186 %182
        %187 = OpPhi %float %float_1 %105 %188 %182
        %189 = OpPhi %float %float_1 %105 %180 %182
        %190 = OpConvertSToF %float %185
        %191 = OpFAdd %float %171 %float_2
        %192 = OpFOrdLessThan %bool %190 %191
               OpLoopMerge %193 %182 None
               OpBranchConditional %192 %194 %193
        %194 = OpLabel
        %195 = OpFAdd %v2float %125 %183
        %196 = OpSampledImage %type_sampled_image %177 %178
        %197 = OpImageSampleExplicitLod %v4float %196 %195 Grad %175 %176
        %188 = OpCompositeExtract %float %197 1
        %198 = OpFOrdLessThan %bool %180 %188
               OpSelectionMerge %182 None
               OpBranchConditional %198 %199 %182
        %199 = OpLabel
        %200 = OpFSub %float %189 %187
        %201 = OpFSub %float %188 %180
        %202 = OpFAdd %float %200 %201
        %203 = OpFDiv %float %201 %202
        %204 = OpFMul %float %189 %203
        %205 = OpFSub %float %float_1 %203
        %206 = OpFMul %float %180 %205
        %207 = OpFAdd %float %204 %206
        %208 = OpCompositeConstruct %v2float %203 %203
        %209 = OpFMul %v2float %208 %174
        %210 = OpFSub %v2float %183 %209
               OpBranch %193
        %182 = OpLabel
        %181 = OpFSub %float %180 %172
        %184 = OpFAdd %v2float %183 %174
        %186 = OpIAdd %int %185 %int_1
               OpBranch %179
        %193 = OpLabel
        %211 = OpPhi %float %98 %179 %207 %199
        %212 = OpPhi %v2float %183 %179 %210 %199
        %213 = OpVectorShuffle %v2float %212 %104 0 1
        %214 = OpFAdd %v2float %125 %213
        %215 = OpAccessChain %_ptr_Uniform_float %View %int_82
        %216 = OpLoad %float %215
        %217 = OpSampledImage %type_sampled_image %177 %178
        %218 = OpImageSampleImplicitLod %v4float %217 %214 Bias %216
        %219 = OpCompositeExtract %float %218 0
        %220 = OpAccessChain %_ptr_Uniform_float %Material %int_1 %int_2 %int_1
        %221 = OpLoad %float %220
        %222 = OpFMul %float %219 %221
        %223 = OpFSub %float %float_1 %222
        %224 = OpExtInst %float %1 FMax %223 %float_0
        %225 = OpExtInst %float %1 FMin %224 %float_1
        %226 = OpAccessChain %_ptr_Uniform_float %View %int_98 %int_0
        %227 = OpLoad %float %226
        %228 = OpCompositeConstruct %v2float %227 %227
        %229 = OpFAdd %v2float %135 %228
        %230 = OpCompositeExtract %float %229 0
        %231 = OpConvertFToU %uint %230
        %232 = OpCompositeExtract %float %229 1
        %233 = OpConvertFToU %uint %232
        %234 = OpIMul %uint %uint_2 %233
        %235 = OpIAdd %uint %231 %234
        %236 = OpUMod %uint %235 %uint_5
        %237 = OpConvertUToF %float %236
        %238 = OpFMul %v2float %135 %101
        %239 = OpLoad %type_2d_image %Material_Texture2D_3
        %240 = OpLoad %type_sampler %Material_Texture2D_3Sampler
        %241 = OpSampledImage %type_sampled_image %239 %240
        %242 = OpImageSampleImplicitLod %v4float %241 %238 Bias %216
        %243 = OpCompositeExtract %float %242 0
        %244 = OpAccessChain %_ptr_Uniform_float %Material %int_1 %int_2 %int_2
        %245 = OpLoad %float %244
        %246 = OpFMul %float %243 %245
        %247 = OpFAdd %float %237 %246
        %248 = OpFMul %float %247 %float_0_166666672
        %249 = OpFAdd %float %225 %248
        %250 = OpFAdd %float %249 %float_n0_5
        %251 = OpCompositeExtract %float %218 2
        %252 = OpFAdd %float %251 %250
        %253 = OpSampledImage %type_sampled_image %239 %240
        %254 = OpImageSampleImplicitLod %v4float %253 %238 Bias %216
        %255 = OpCompositeExtract %float %254 0
        %256 = OpFAdd %float %237 %255
        %257 = OpFMul %float %256 %float_0_166666672
        %258 = OpAccessChain %_ptr_Uniform_float %PrimitiveFade %int_0 %int_0
        %259 = OpLoad %float %258
        %260 = OpFMul %float %123 %259
        %261 = OpAccessChain %_ptr_Uniform_float %PrimitiveFade %int_0 %int_1
        %262 = OpLoad %float %261
        %263 = OpFAdd %float %260 %262
        %264 = OpExtInst %float %1 FClamp %263 %float_0 %float_1
        %265 = OpFAdd %float %264 %257
        %266 = OpFAdd %float %265 %float_n0_5
        %267 = OpFMul %float %252 %266
        %268 = OpFSub %float %float_1 %211
        %269 = OpFMul %float %268 %159
        %270 = OpCompositeExtract %float %212 0
        %271 = OpCompositeExtract %float %212 1
        %272 = OpCompositeConstruct %v3float %270 %271 %269
        %273 = OpDot %float %272 %272
        %274 = OpExtInst %float %1 Sqrt %273
        %275 = OpDPdx %v2float %125
        %276 = OpExtInst %v2float %1 FAbs %275
        %277 = OpDot %float %276 %276
        %278 = OpExtInst %float %1 Sqrt %277
        %279 = OpDPdx %v3float %149
        %280 = OpDot %float %279 %279
        %281 = OpExtInst %float %1 Sqrt %280
        %282 = OpFDiv %float %278 %281
        %283 = OpDPdy %v2float %125
        %284 = OpExtInst %v2float %1 FAbs %283
        %285 = OpDot %float %284 %284
        %286 = OpExtInst %float %1 Sqrt %285
        %287 = OpDPdy %v3float %149
        %288 = OpDot %float %287 %287
        %289 = OpExtInst %float %1 Sqrt %288
        %290 = OpFDiv %float %286 %289
        %291 = OpExtInst %float %1 FMax %282 %290
        %292 = OpCompositeExtract %v4float %113 0
        %293 = OpVectorShuffle %v3float %292 %292 0 1 2
        %294 = OpCompositeExtract %v4float %113 1
        %295 = OpVectorShuffle %v3float %294 %294 0 1 2
        %296 = OpCompositeExtract %v4float %113 2
        %297 = OpVectorShuffle %v3float %296 %296 0 1 2
        %298 = OpCompositeConstruct %mat3v3float %293 %295 %297
        %299 = OpMatrixTimesVector %v3float %298 %70
        %300 = OpDot %float %299 %151
        %301 = OpExtInst %float %1 FAbs %300
        %302 = OpFDiv %float %291 %301
        %303 = OpFDiv %float %274 %302
        %304 = OpAccessChain %_ptr_Uniform_float %PrimitiveDither %int_0
        %305 = OpLoad %float %304
        %306 = OpFOrdNotEqual %bool %305 %float_0
               OpSelectionMerge %307 None
               OpBranchConditional %306 %308 %307
        %308 = OpLabel
        %309 = OpExtInst %float %1 FAbs %305
        %310 = OpFOrdGreaterThan %bool %309 %float_0_00100000005
               OpSelectionMerge %311 None
               OpBranchConditional %310 %312 %311
        %312 = OpLabel
        %313 = OpExtInst %v2float %1 Floor %133
        %314 = OpDot %float %313 %75
        %315 = OpExtInst %float %1 Cos %314
        %316 = OpFMul %float %315 %float_1000
        %317 = OpExtInst %float %1 Fract %316
        %318 = OpFOrdLessThan %bool %305 %float_0
        %319 = OpFAdd %float %305 %float_1
        %320 = OpFOrdGreaterThan %bool %319 %317
        %321 = OpFOrdLessThan %bool %305 %317
        %322 = OpSelect %bool %318 %320 %321
        %323 = OpSelect %float %322 %float_1 %float_0
        %324 = OpFSub %float %323 %float_0_00100000005
        %325 = OpFOrdLessThan %bool %324 %float_0
               OpSelectionMerge %326 None
               OpBranchConditional %325 %327 %326
        %327 = OpLabel
               OpKill
        %326 = OpLabel
               OpBranch %311
        %311 = OpLabel
               OpBranch %307
        %307 = OpLabel
        %328 = OpFSub %float %267 %float_0_333299994
        %329 = OpFOrdLessThan %bool %328 %float_0
               OpSelectionMerge %330 None
               OpBranchConditional %329 %331 %330
        %331 = OpLabel
               OpKill
        %330 = OpLabel
        %332 = OpCompositeExtract %float %140 2
        %333 = OpCompositeExtract %float %140 3
        %334 = OpFAdd %float %333 %303
        %335 = OpFDiv %float %332 %334
        %336 = OpExtInst %float %1 FMin %335 %136
        %337 = OpVectorShuffle %v2float %107 %107 0 1
        %338 = OpCompositeExtract %float %107 3
        %339 = OpCompositeConstruct %v2float %338 %338
        %340 = OpFDiv %v2float %337 %339
        %341 = OpVectorShuffle %v2float %119 %119 0 1
        %342 = OpFSub %v2float %340 %341
        %343 = OpVectorShuffle %v2float %108 %108 0 1
        %344 = OpCompositeExtract %float %108 3
        %345 = OpCompositeConstruct %v2float %344 %344
        %346 = OpFDiv %v2float %343 %345
        %347 = OpVectorShuffle %v2float %119 %119 2 3
        %348 = OpFSub %v2float %346 %347
        %349 = OpFSub %v2float %342 %348
        %350 = OpFMul %v2float %349 %54
        %351 = OpFAdd %v2float %350 %56
        %352 = OpVectorShuffle %v4float %351 %49 0 1 2 3
               OpStore %gl_FragDepth %336
               OpStore %out_var_SV_Target0 %352
               OpReturn
               OpFunctionEnd
