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
    float PrePadding_View_3048;
    float PrePadding_View_3052;
    float4x4 View_WorldToVirtualTexture;
    float4 View_VirtualTextureParams;
    float4 View_XRPassthroughCameraUVs[2];
};

constant float4 _68 = {};

struct main0_out
{
    float4 out_var_TEXCOORD10_centroid [[user(locn0)]];
    float4 out_var_TEXCOORD11_centroid [[user(locn1)]];
    float4 out_var_TEXCOORD0_0 [[user(locn2)]];
    float4 out_var_COLOR1 [[user(locn3)]];
    float4 out_var_COLOR2 [[user(locn4)]];
    float4 out_var_TEXCOORD6 [[user(locn5)]];
    float3 out_var_TEXCOORD7 [[user(locn6)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 in_var_COLOR1 [[attribute(0)]];
    float4 in_var_COLOR2 [[attribute(1)]];
    float4 in_var_TEXCOORD0_0 [[attribute(5)]];
    float4 in_var_TEXCOORD10_centroid [[attribute(6)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(7)]];
    float3 in_var_TEXCOORD7 [[attribute(8)]];
    float4 in_var_VS_To_DS_Position [[attribute(9)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant type_View& View [[buffer(0)]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 1> out_var_TEXCOORD0 = {};
    spvUnsafeArray<float4, 3> _77 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_TEXCOORD10_centroid, patchIn.gl_in[1].in_var_TEXCOORD10_centroid, patchIn.gl_in[2].in_var_TEXCOORD10_centroid });
    spvUnsafeArray<float4, 3> _78 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_TEXCOORD11_centroid, patchIn.gl_in[1].in_var_TEXCOORD11_centroid, patchIn.gl_in[2].in_var_TEXCOORD11_centroid });
    spvUnsafeArray<spvUnsafeArray<float4, 1>, 3> _79 = spvUnsafeArray<spvUnsafeArray<float4, 1>, 3>({ spvUnsafeArray<float4, 1>({ patchIn.gl_in[0].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ patchIn.gl_in[1].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ patchIn.gl_in[2].in_var_TEXCOORD0_0 }) });
    spvUnsafeArray<float4, 3> _80 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_COLOR1, patchIn.gl_in[1].in_var_COLOR1, patchIn.gl_in[2].in_var_COLOR1 });
    spvUnsafeArray<float4, 3> _81 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_COLOR2, patchIn.gl_in[1].in_var_COLOR2, patchIn.gl_in[2].in_var_COLOR2 });
    spvUnsafeArray<float4, 3> _97 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_VS_To_DS_Position, patchIn.gl_in[1].in_var_VS_To_DS_Position, patchIn.gl_in[2].in_var_VS_To_DS_Position });
    spvUnsafeArray<float3, 3> _98 = spvUnsafeArray<float3, 3>({ patchIn.gl_in[0].in_var_TEXCOORD7, patchIn.gl_in[1].in_var_TEXCOORD7, patchIn.gl_in[2].in_var_TEXCOORD7 });
    float4 _111 = float4(gl_TessCoord.x);
    float4 _113 = float4(gl_TessCoord.y);
    float4 _116 = float4(gl_TessCoord.z);
    float4 _118 = ((_97[0] * _111) + (_97[1] * _113)) + (_97[2] * _116);
    spvUnsafeArray<float4, 1> _72 = _79[0];
    spvUnsafeArray<float4, 1> _71 = _79[1];
    float3 _120 = float3(gl_TessCoord.x);
    float3 _123 = float3(gl_TessCoord.y);
    spvUnsafeArray<float4, 1> _73;
    for (int _133 = 0; _133 < 1; )
    {
        _73[_133] = (_72[_133] * _111) + (_71[_133] * _113);
        _133++;
        continue;
    }
    spvUnsafeArray<float4, 1> _75 = _73;
    spvUnsafeArray<float4, 1> _74 = _79[2];
    float3 _155 = float3(gl_TessCoord.z);
    float3 _157 = ((_77[0].xyz * _120) + (_77[1].xyz * _123)).xyz + (_77[2].xyz * _155);
    spvUnsafeArray<float4, 1> _76;
    for (int _164 = 0; _164 < 1; )
    {
        _76[_164] = _75[_164] + (_74[_164] * _116);
        _164++;
        continue;
    }
    float4 _181 = float4(_118.x, _118.y, _118.z, _118.w);
    out.out_var_TEXCOORD10_centroid = float4(_157.x, _157.y, _157.z, _68.w);
    out.out_var_TEXCOORD11_centroid = ((_78[0] * _111) + (_78[1] * _113)) + (_78[2] * _116);
    out_var_TEXCOORD0 = _76;
    out.out_var_COLOR1 = ((_80[0] * _111) + (_80[1] * _113)) + (_80[2] * _116);
    out.out_var_COLOR2 = ((_81[0] * _111) + (_81[1] * _113)) + (_81[2] * _116);
    out.out_var_TEXCOORD6 = _181;
    out.out_var_TEXCOORD7 = ((_98[0] * _120) + (_98[1] * _123)) + (_98[2] * _155);
    out.gl_Position = View.View_TranslatedWorldToClip * _181;
    out.out_var_TEXCOORD0_0 = out_var_TEXCOORD0[0];
    return out;
}

