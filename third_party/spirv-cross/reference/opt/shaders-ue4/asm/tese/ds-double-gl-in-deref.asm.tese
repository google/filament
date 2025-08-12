#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct type_View
{
    float4x4 View_TranslatedWorldToClip;
    float4x4 View_WorldToClip;
    float4x4 View_ClipToWorld;
    float4x4 View_TranslatedWorldToView;
    float4x4 View_ViewToTranslatedWorld;
    float4x4 View_TranslatedWorldToCameraView;
    float4x4 View_CameraViewToTranslatedWorld;
    float4x4 View_ViewToClip;
    float4x4 View_ViewToClipNoAA;
    float4x4 View_ClipToView;
    float4x4 View_ClipToTranslatedWorld;
    float4x4 View_SVPositionToTranslatedWorld;
    float4x4 View_ScreenToWorld;
    float4x4 View_ScreenToTranslatedWorld;
    packed_float3 View_ViewForward;
    float PrePadding_View_908;
    packed_float3 View_ViewUp;
    float PrePadding_View_924;
    packed_float3 View_ViewRight;
    float PrePadding_View_940;
    packed_float3 View_HMDViewNoRollUp;
    float PrePadding_View_956;
    packed_float3 View_HMDViewNoRollRight;
    float PrePadding_View_972;
    float4 View_InvDeviceZToWorldZTransform;
    float4 View_ScreenPositionScaleBias;
    packed_float3 View_WorldCameraOrigin;
    float PrePadding_View_1020;
    packed_float3 View_TranslatedWorldCameraOrigin;
    float PrePadding_View_1036;
    packed_float3 View_WorldViewOrigin;
    float PrePadding_View_1052;
    packed_float3 View_PreViewTranslation;
    float PrePadding_View_1068;
    float4x4 View_PrevProjection;
    float4x4 View_PrevViewProj;
    float4x4 View_PrevViewRotationProj;
    float4x4 View_PrevViewToClip;
    float4x4 View_PrevClipToView;
    float4x4 View_PrevTranslatedWorldToClip;
    float4x4 View_PrevTranslatedWorldToView;
    float4x4 View_PrevViewToTranslatedWorld;
    float4x4 View_PrevTranslatedWorldToCameraView;
    float4x4 View_PrevCameraViewToTranslatedWorld;
    packed_float3 View_PrevWorldCameraOrigin;
    float PrePadding_View_1724;
    packed_float3 View_PrevWorldViewOrigin;
    float PrePadding_View_1740;
    packed_float3 View_PrevPreViewTranslation;
    float PrePadding_View_1756;
    float4x4 View_PrevInvViewProj;
    float4x4 View_PrevScreenToTranslatedWorld;
    float4x4 View_ClipToPrevClip;
    float4 View_TemporalAAJitter;
    float4 View_GlobalClippingPlane;
    float2 View_FieldOfViewWideAngles;
    float2 View_PrevFieldOfViewWideAngles;
    float4 View_ViewRectMin;
    float4 View_ViewSizeAndInvSize;
    float4 View_BufferSizeAndInvSize;
    float4 View_BufferBilinearUVMinMax;
    int View_NumSceneColorMSAASamples;
    float View_PreExposure;
    float View_OneOverPreExposure;
    float PrePadding_View_2076;
    float4 View_DiffuseOverrideParameter;
    float4 View_SpecularOverrideParameter;
    float4 View_NormalOverrideParameter;
    float2 View_RoughnessOverrideParameter;
    float View_PrevFrameGameTime;
    float View_PrevFrameRealTime;
    float View_OutOfBoundsMask;
    float PrePadding_View_2148;
    float PrePadding_View_2152;
    float PrePadding_View_2156;
    packed_float3 View_WorldCameraMovementSinceLastFrame;
    float View_CullingSign;
    float View_NearPlane;
    float View_AdaptiveTessellationFactor;
    float View_GameTime;
    float View_RealTime;
    float View_DeltaTime;
    float View_MaterialTextureMipBias;
    float View_MaterialTextureDerivativeMultiply;
    uint View_Random;
    uint View_FrameNumber;
    uint View_StateFrameIndexMod8;
    uint View_StateFrameIndex;
    float View_CameraCut;
    float View_UnlitViewmodeMask;
    float PrePadding_View_2228;
    float PrePadding_View_2232;
    float PrePadding_View_2236;
    float4 View_DirectionalLightColor;
    packed_float3 View_DirectionalLightDirection;
    float PrePadding_View_2268;
    float4 View_TranslucencyLightingVolumeMin[2];
    float4 View_TranslucencyLightingVolumeInvSize[2];
    float4 View_TemporalAAParams;
    float4 View_CircleDOFParams;
    float View_DepthOfFieldSensorWidth;
    float View_DepthOfFieldFocalDistance;
    float View_DepthOfFieldScale;
    float View_DepthOfFieldFocalLength;
    float View_DepthOfFieldFocalRegion;
    float View_DepthOfFieldNearTransitionRegion;
    float View_DepthOfFieldFarTransitionRegion;
    float View_MotionBlurNormalizedToPixel;
    float View_bSubsurfacePostprocessEnabled;
    float View_GeneralPurposeTweak;
    float View_DemosaicVposOffset;
    float PrePadding_View_2412;
    packed_float3 View_IndirectLightingColorScale;
    float View_HDR32bppEncodingMode;
    packed_float3 View_AtmosphericFogSunDirection;
    float View_AtmosphericFogSunPower;
    float View_AtmosphericFogPower;
    float View_AtmosphericFogDensityScale;
    float View_AtmosphericFogDensityOffset;
    float View_AtmosphericFogGroundOffset;
    float View_AtmosphericFogDistanceScale;
    float View_AtmosphericFogAltitudeScale;
    float View_AtmosphericFogHeightScaleRayleigh;
    float View_AtmosphericFogStartDistance;
    float View_AtmosphericFogDistanceOffset;
    float View_AtmosphericFogSunDiscScale;
    float View_AtmosphericFogSunDiscHalfApexAngleRadian;
    float PrePadding_View_2492;
    float4 View_AtmosphericFogSunDiscLuminance;
    uint View_AtmosphericFogRenderMask;
    uint View_AtmosphericFogInscatterAltitudeSampleNum;
    uint PrePadding_View_2520;
    uint PrePadding_View_2524;
    float4 View_AtmosphericFogSunColor;
    packed_float3 View_NormalCurvatureToRoughnessScaleBias;
    float View_RenderingReflectionCaptureMask;
    float4 View_AmbientCubemapTint;
    float View_AmbientCubemapIntensity;
    float View_SkyLightParameters;
    float PrePadding_View_2584;
    float PrePadding_View_2588;
    float4 View_SkyLightColor;
    float4 View_SkyIrradianceEnvironmentMap[7];
    float View_MobilePreviewMode;
    float View_HMDEyePaddingOffset;
    float View_ReflectionCubemapMaxMip;
    float View_ShowDecalsMask;
    uint View_DistanceFieldAOSpecularOcclusionMode;
    float View_IndirectCapsuleSelfShadowingIntensity;
    float PrePadding_View_2744;
    float PrePadding_View_2748;
    packed_float3 View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight;
    int View_StereoPassIndex;
    float4 View_GlobalVolumeCenterAndExtent[4];
    float4 View_GlobalVolumeWorldToUVAddAndMul[4];
    float View_GlobalVolumeDimension;
    float View_GlobalVolumeTexelSize;
    float View_MaxGlobalDistance;
    float PrePadding_View_2908;
    int2 View_CursorPosition;
    float View_bCheckerboardSubsurfaceProfileRendering;
    float PrePadding_View_2924;
    packed_float3 View_VolumetricFogInvGridSize;
    float PrePadding_View_2940;
    packed_float3 View_VolumetricFogGridZParams;
    float PrePadding_View_2956;
    float2 View_VolumetricFogSVPosToVolumeUV;
    float View_VolumetricFogMaxDistance;
    float PrePadding_View_2972;
    packed_float3 View_VolumetricLightmapWorldToUVScale;
    float PrePadding_View_2988;
    packed_float3 View_VolumetricLightmapWorldToUVAdd;
    float PrePadding_View_3004;
    packed_float3 View_VolumetricLightmapIndirectionTextureSize;
    float View_VolumetricLightmapBrickSize;
    packed_float3 View_VolumetricLightmapBrickTexelSize;
    float View_StereoIPD;
    float View_IndirectLightingCacheShowFlag;
    float View_EyeToPixelSpreadAngle;
};

struct type_ShadowDepthPass
{
    float PrePadding_ShadowDepthPass_LPV_0;
    float PrePadding_ShadowDepthPass_LPV_4;
    float PrePadding_ShadowDepthPass_LPV_8;
    float PrePadding_ShadowDepthPass_LPV_12;
    float PrePadding_ShadowDepthPass_LPV_16;
    float PrePadding_ShadowDepthPass_LPV_20;
    float PrePadding_ShadowDepthPass_LPV_24;
    float PrePadding_ShadowDepthPass_LPV_28;
    float PrePadding_ShadowDepthPass_LPV_32;
    float PrePadding_ShadowDepthPass_LPV_36;
    float PrePadding_ShadowDepthPass_LPV_40;
    float PrePadding_ShadowDepthPass_LPV_44;
    float PrePadding_ShadowDepthPass_LPV_48;
    float PrePadding_ShadowDepthPass_LPV_52;
    float PrePadding_ShadowDepthPass_LPV_56;
    float PrePadding_ShadowDepthPass_LPV_60;
    float PrePadding_ShadowDepthPass_LPV_64;
    float PrePadding_ShadowDepthPass_LPV_68;
    float PrePadding_ShadowDepthPass_LPV_72;
    float PrePadding_ShadowDepthPass_LPV_76;
    float PrePadding_ShadowDepthPass_LPV_80;
    float PrePadding_ShadowDepthPass_LPV_84;
    float PrePadding_ShadowDepthPass_LPV_88;
    float PrePadding_ShadowDepthPass_LPV_92;
    float PrePadding_ShadowDepthPass_LPV_96;
    float PrePadding_ShadowDepthPass_LPV_100;
    float PrePadding_ShadowDepthPass_LPV_104;
    float PrePadding_ShadowDepthPass_LPV_108;
    float PrePadding_ShadowDepthPass_LPV_112;
    float PrePadding_ShadowDepthPass_LPV_116;
    float PrePadding_ShadowDepthPass_LPV_120;
    float PrePadding_ShadowDepthPass_LPV_124;
    float PrePadding_ShadowDepthPass_LPV_128;
    float PrePadding_ShadowDepthPass_LPV_132;
    float PrePadding_ShadowDepthPass_LPV_136;
    float PrePadding_ShadowDepthPass_LPV_140;
    float PrePadding_ShadowDepthPass_LPV_144;
    float PrePadding_ShadowDepthPass_LPV_148;
    float PrePadding_ShadowDepthPass_LPV_152;
    float PrePadding_ShadowDepthPass_LPV_156;
    float PrePadding_ShadowDepthPass_LPV_160;
    float PrePadding_ShadowDepthPass_LPV_164;
    float PrePadding_ShadowDepthPass_LPV_168;
    float PrePadding_ShadowDepthPass_LPV_172;
    float PrePadding_ShadowDepthPass_LPV_176;
    float PrePadding_ShadowDepthPass_LPV_180;
    float PrePadding_ShadowDepthPass_LPV_184;
    float PrePadding_ShadowDepthPass_LPV_188;
    float PrePadding_ShadowDepthPass_LPV_192;
    float PrePadding_ShadowDepthPass_LPV_196;
    float PrePadding_ShadowDepthPass_LPV_200;
    float PrePadding_ShadowDepthPass_LPV_204;
    float PrePadding_ShadowDepthPass_LPV_208;
    float PrePadding_ShadowDepthPass_LPV_212;
    float PrePadding_ShadowDepthPass_LPV_216;
    float PrePadding_ShadowDepthPass_LPV_220;
    float PrePadding_ShadowDepthPass_LPV_224;
    float PrePadding_ShadowDepthPass_LPV_228;
    float PrePadding_ShadowDepthPass_LPV_232;
    float PrePadding_ShadowDepthPass_LPV_236;
    float PrePadding_ShadowDepthPass_LPV_240;
    float PrePadding_ShadowDepthPass_LPV_244;
    float PrePadding_ShadowDepthPass_LPV_248;
    float PrePadding_ShadowDepthPass_LPV_252;
    float PrePadding_ShadowDepthPass_LPV_256;
    float PrePadding_ShadowDepthPass_LPV_260;
    float PrePadding_ShadowDepthPass_LPV_264;
    float PrePadding_ShadowDepthPass_LPV_268;
    float4x4 ShadowDepthPass_LPV_mRsmToWorld;
    float4 ShadowDepthPass_LPV_mLightColour;
    float4 ShadowDepthPass_LPV_GeometryVolumeCaptureLightDirection;
    float4 ShadowDepthPass_LPV_mEyePos;
    packed_int3 ShadowDepthPass_LPV_mOldGridOffset;
    int PrePadding_ShadowDepthPass_LPV_396;
    packed_int3 ShadowDepthPass_LPV_mLpvGridOffset;
    float ShadowDepthPass_LPV_ClearMultiplier;
    float ShadowDepthPass_LPV_LpvScale;
    float ShadowDepthPass_LPV_OneOverLpvScale;
    float ShadowDepthPass_LPV_DirectionalOcclusionIntensity;
    float ShadowDepthPass_LPV_DirectionalOcclusionRadius;
    float ShadowDepthPass_LPV_RsmAreaIntensityMultiplier;
    float ShadowDepthPass_LPV_RsmPixelToTexcoordMultiplier;
    float ShadowDepthPass_LPV_SecondaryOcclusionStrength;
    float ShadowDepthPass_LPV_SecondaryBounceStrength;
    float ShadowDepthPass_LPV_VplInjectionBias;
    float ShadowDepthPass_LPV_GeometryVolumeInjectionBias;
    float ShadowDepthPass_LPV_EmissiveInjectionMultiplier;
    int ShadowDepthPass_LPV_PropagationIndex;
    float4x4 ShadowDepthPass_ProjectionMatrix;
    float4x4 ShadowDepthPass_ViewMatrix;
    float4 ShadowDepthPass_ShadowParams;
    float ShadowDepthPass_bClampToNearPlane;
    float PrePadding_ShadowDepthPass_612;
    float PrePadding_ShadowDepthPass_616;
    float PrePadding_ShadowDepthPass_620;
    float4x4 ShadowDepthPass_ShadowViewProjectionMatrices[6];
    float4x4 ShadowDepthPass_ShadowViewMatrices[6];
};

constant float4 _113 = {};

struct main0_out
{
    float4 out_var_TEXCOORD10_centroid [[user(locn0)]];
    float4 out_var_TEXCOORD11_centroid [[user(locn1)]];
    float4 out_var_COLOR0 [[user(locn2)]];
    float4 out_var_TEXCOORD0_0 [[user(locn3)]];
    uint out_var_PRIMITIVE_ID [[user(locn4)]];
    float out_var_TEXCOORD6 [[user(locn5)]];
    float out_var_TEXCOORD8 [[user(locn6)]];
    float3 out_var_TEXCOORD7 [[user(locn7)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 in_var_COLOR0 [[attribute(0)]];
    float4 in_var_PN_POSITION_0 [[attribute(2)]];
    float4 in_var_PN_POSITION_1 [[attribute(3)]];
    float4 in_var_PN_POSITION_2 [[attribute(4)]];
    float in_var_PN_WorldDisplacementMultiplier [[attribute(7)]];
    uint in_var_PRIMITIVE_ID [[attribute(8)]];
    float4 in_var_TEXCOORD0_0 [[attribute(9)]];
    float4 in_var_TEXCOORD10_centroid [[attribute(10)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(11)]];
};

struct main0_patchIn
{
    float4 in_var_PN_POSITION9 [[attribute(5)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant type_View& View [[buffer(0)]], constant type_ShadowDepthPass& ShadowDepthPass [[buffer(1)]], texture2d<float> Material_Texture2D_3 [[texture(0)]], sampler Material_Texture2D_3Sampler [[sampler(0)]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 1> out_var_TEXCOORD0 = {};
    spvUnsafeArray<float4, 3> _117 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_TEXCOORD10_centroid, patchIn.gl_in[1].in_var_TEXCOORD10_centroid, patchIn.gl_in[2].in_var_TEXCOORD10_centroid });
    spvUnsafeArray<float4, 3> _118 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_TEXCOORD11_centroid, patchIn.gl_in[1].in_var_TEXCOORD11_centroid, patchIn.gl_in[2].in_var_TEXCOORD11_centroid });
    spvUnsafeArray<float4, 3> _119 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_COLOR0, patchIn.gl_in[1].in_var_COLOR0, patchIn.gl_in[2].in_var_COLOR0 });
    spvUnsafeArray<spvUnsafeArray<float4, 1>, 3> _120 = spvUnsafeArray<spvUnsafeArray<float4, 1>, 3>({ spvUnsafeArray<float4, 1>({ patchIn.gl_in[0].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ patchIn.gl_in[1].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ patchIn.gl_in[2].in_var_TEXCOORD0_0 }) });
    spvUnsafeArray<spvUnsafeArray<float4, 3>, 3> _135 = spvUnsafeArray<spvUnsafeArray<float4, 3>, 3>({ spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_PN_POSITION_0, patchIn.gl_in[0].in_var_PN_POSITION_1, patchIn.gl_in[0].in_var_PN_POSITION_2 }), spvUnsafeArray<float4, 3>({ patchIn.gl_in[1].in_var_PN_POSITION_0, patchIn.gl_in[1].in_var_PN_POSITION_1, patchIn.gl_in[1].in_var_PN_POSITION_2 }), spvUnsafeArray<float4, 3>({ patchIn.gl_in[2].in_var_PN_POSITION_0, patchIn.gl_in[2].in_var_PN_POSITION_1, patchIn.gl_in[2].in_var_PN_POSITION_2 }) });
    spvUnsafeArray<float, 3> _136 = spvUnsafeArray<float, 3>({ patchIn.gl_in[0].in_var_PN_WorldDisplacementMultiplier, patchIn.gl_in[1].in_var_PN_WorldDisplacementMultiplier, patchIn.gl_in[2].in_var_PN_WorldDisplacementMultiplier });
    float _157 = gl_TessCoord.x * gl_TessCoord.x;
    float _158 = gl_TessCoord.y * gl_TessCoord.y;
    float _159 = gl_TessCoord.z * gl_TessCoord.z;
    float4 _165 = float4(gl_TessCoord.x);
    float4 _169 = float4(gl_TessCoord.y);
    float4 _174 = float4(gl_TessCoord.z);
    float4 _177 = float4(_157 * 3.0);
    float4 _181 = float4(_158 * 3.0);
    float4 _188 = float4(_159 * 3.0);
    float4 _202 = fma(((patchIn.in_var_PN_POSITION9 * float4(6.0)) * _174) * _165, _169, fma(_135[2][2] * _177, _174, fma(_135[2][1] * _188, _165, fma(_135[1][2] * _188, _169, fma(_135[1][1] * _181, _174, fma(_135[0][2] * _181, _165, fma(_135[0][1] * _177, _169, fma(_135[2][0] * float4(_159), _174, fma(_135[0][0] * float4(_157), _165, (_135[1][0] * float4(_158)) * _169)))))))));
    float3 _226 = fma(_117[2].xyz, float3(gl_TessCoord.z), fma(_117[0].xyz, float3(gl_TessCoord.x), _117[1].xyz * float3(gl_TessCoord.y)).xyz);
    float4 _229 = fma(_118[2], _174, fma(_118[0], _165, _118[1] * _169));
    float4 _231 = fma(_119[2], _174, fma(_119[0], _165, _119[1] * _169));
    float4 _233 = fma(_120[2][0], _174, fma(_120[0][0], _165, _120[1][0] * _169));
    spvUnsafeArray<float4, 1> _234 = spvUnsafeArray<float4, 1>({ _233 });
    float3 _236 = _229.xyz;
    float3 _264 = fma((float3((Material_Texture2D_3.sample(Material_Texture2D_3Sampler, fma(_233.zw, float2(1.0, 2.0), float2(View.View_GameTime * 0.20000000298023223876953125, View.View_GameTime * (-0.699999988079071044921875))), level(-1.0)).x * 10.0) * (1.0 - _231.x)) * _236) * float3(0.5), float3(fma(_136[2], gl_TessCoord.z, fma(_136[0], gl_TessCoord.x, _136[1] * gl_TessCoord.y))), _202.xyz);
    float4 _270 = ShadowDepthPass.ShadowDepthPass_ProjectionMatrix * float4(_264.x, _264.y, _264.z, _202.w);
    float4 _281;
    if ((ShadowDepthPass.ShadowDepthPass_bClampToNearPlane > 0.0) && (_270.z < 0.0))
    {
        float4 _279 = _270;
        _279.z = 9.9999999747524270787835121154785e-07;
        _279.w = 1.0;
        _281 = _279;
    }
    else
    {
        _281 = _270;
    }
    float _290 = abs(dot(float3(ShadowDepthPass.ShadowDepthPass_ViewMatrix[0].z, ShadowDepthPass.ShadowDepthPass_ViewMatrix[1].z, ShadowDepthPass.ShadowDepthPass_ViewMatrix[2].z), _236));
    out.out_var_TEXCOORD10_centroid = float4(_226.x, _226.y, _226.z, _113.w);
    out.out_var_TEXCOORD11_centroid = _229;
    out.out_var_COLOR0 = _231;
    out_var_TEXCOORD0 = _234;
    out.out_var_PRIMITIVE_ID = patchIn.gl_in[0u].in_var_PRIMITIVE_ID;
    out.out_var_TEXCOORD6 = _281.z;
    out.out_var_TEXCOORD8 = fma(ShadowDepthPass.ShadowDepthPass_ShadowParams.y, fast::clamp((abs(_290) > 0.0) ? (sqrt(fast::clamp(fma(-_290, _290, 1.0), 0.0, 1.0)) / _290) : ShadowDepthPass.ShadowDepthPass_ShadowParams.z, 0.0, ShadowDepthPass.ShadowDepthPass_ShadowParams.z), ShadowDepthPass.ShadowDepthPass_ShadowParams.x);
    out.out_var_TEXCOORD7 = _264.xyz;
    out.gl_Position = _281;
    out.out_var_TEXCOORD0_0 = out_var_TEXCOORD0[0];
    return out;
}

