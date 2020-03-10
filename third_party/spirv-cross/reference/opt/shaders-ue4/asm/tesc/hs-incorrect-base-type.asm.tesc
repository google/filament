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

struct FVertexFactoryInterpolantsVSToPS
{
    float4 TangentToWorld0;
    float4 TangentToWorld2;
    float4 Color;
    spvUnsafeArray<float4, 1> TexCoords;
    float4 LightMapCoordinate;
    uint PrimitiveId;
    uint LightmapDataIndex;
};

struct FVertexFactoryInterpolantsVSToDS
{
    FVertexFactoryInterpolantsVSToPS InterpolantsVSToPS;
};

struct FSharedBasePassInterpolants
{
};
struct FBasePassInterpolantsVSToDS
{
    FSharedBasePassInterpolants _m0;
};

struct FBasePassVSToDS
{
    FVertexFactoryInterpolantsVSToDS FactoryInterpolants;
    FBasePassInterpolantsVSToDS BasePassInterpolants;
    float4 Position;
};

struct FPNTessellationHSToDS
{
    FBasePassVSToDS PassSpecificData;
    spvUnsafeArray<float4, 3> WorldPosition;
    float3 DisplacementScale;
    float TessellationMultiplier;
    float WorldDisplacementMultiplier;
};

struct type_View
{
    float4x4 View_TranslatedWorldToClip;
    float4x4 View_WorldToClip;
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
    float PrePadding_View_844;
    packed_float3 View_ViewUp;
    float PrePadding_View_860;
    packed_float3 View_ViewRight;
    float PrePadding_View_876;
    packed_float3 View_HMDViewNoRollUp;
    float PrePadding_View_892;
    packed_float3 View_HMDViewNoRollRight;
    float PrePadding_View_908;
    float4 View_InvDeviceZToWorldZTransform;
    float4 View_ScreenPositionScaleBias;
    packed_float3 View_WorldCameraOrigin;
    float PrePadding_View_956;
    packed_float3 View_TranslatedWorldCameraOrigin;
    float PrePadding_View_972;
    packed_float3 View_WorldViewOrigin;
    float PrePadding_View_988;
    packed_float3 View_PreViewTranslation;
    float PrePadding_View_1004;
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
    float PrePadding_View_1660;
    packed_float3 View_PrevWorldViewOrigin;
    float PrePadding_View_1676;
    packed_float3 View_PrevPreViewTranslation;
    float PrePadding_View_1692;
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
    float PrePadding_View_2012;
    float4 View_DiffuseOverrideParameter;
    float4 View_SpecularOverrideParameter;
    float4 View_NormalOverrideParameter;
    float2 View_RoughnessOverrideParameter;
    float View_PrevFrameGameTime;
    float View_PrevFrameRealTime;
    float View_OutOfBoundsMask;
    float PrePadding_View_2084;
    float PrePadding_View_2088;
    float PrePadding_View_2092;
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
    float PrePadding_View_2164;
    float PrePadding_View_2168;
    float PrePadding_View_2172;
    float4 View_DirectionalLightColor;
    packed_float3 View_DirectionalLightDirection;
    float PrePadding_View_2204;
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
    float PrePadding_View_2348;
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
    uint View_AtmosphericFogRenderMask;
    uint View_AtmosphericFogInscatterAltitudeSampleNum;
    float4 View_AtmosphericFogSunColor;
    packed_float3 View_NormalCurvatureToRoughnessScaleBias;
    float View_RenderingReflectionCaptureMask;
    float4 View_AmbientCubemapTint;
    float View_AmbientCubemapIntensity;
    float View_SkyLightParameters;
    float PrePadding_View_2488;
    float PrePadding_View_2492;
    float4 View_SkyLightColor;
    float4 View_SkyIrradianceEnvironmentMap[7];
    float View_MobilePreviewMode;
    float View_HMDEyePaddingOffset;
    float View_ReflectionCubemapMaxMip;
    float View_ShowDecalsMask;
    uint View_DistanceFieldAOSpecularOcclusionMode;
    float View_IndirectCapsuleSelfShadowingIntensity;
    float PrePadding_View_2648;
    float PrePadding_View_2652;
    packed_float3 View_ReflectionEnvironmentRoughnessMixingScaleBiasAndLargestWeight;
    int View_StereoPassIndex;
    float4 View_GlobalVolumeCenterAndExtent[4];
    float4 View_GlobalVolumeWorldToUVAddAndMul[4];
    float View_GlobalVolumeDimension;
    float View_GlobalVolumeTexelSize;
    float View_MaxGlobalDistance;
    float View_bCheckerboardSubsurfaceProfileRendering;
    packed_float3 View_VolumetricFogInvGridSize;
    float PrePadding_View_2828;
    packed_float3 View_VolumetricFogGridZParams;
    float PrePadding_View_2844;
    float2 View_VolumetricFogSVPosToVolumeUV;
    float View_VolumetricFogMaxDistance;
    float PrePadding_View_2860;
    packed_float3 View_VolumetricLightmapWorldToUVScale;
    float PrePadding_View_2876;
    packed_float3 View_VolumetricLightmapWorldToUVAdd;
    float PrePadding_View_2892;
    packed_float3 View_VolumetricLightmapIndirectionTextureSize;
    float View_VolumetricLightmapBrickSize;
    packed_float3 View_VolumetricLightmapBrickTexelSize;
    float View_StereoIPD;
    float View_IndirectLightingCacheShowFlag;
    float View_EyeToPixelSpreadAngle;
};

struct type_StructuredBuffer_v4float
{
    float4 _m0[1];
};

constant float4 _602 = {};

struct main0_out
{
    float4 out_var_COLOR0;
    uint out_var_LIGHTMAP_ID;
    float3 out_var_PN_DisplacementScales;
    spvUnsafeArray<float4, 3> out_var_PN_POSITION;
    float out_var_PN_TessellationMultiplier;
    float out_var_PN_WorldDisplacementMultiplier;
    uint out_var_PRIMITIVE_ID;
    spvUnsafeArray<float4, 1> out_var_TEXCOORD0;
    float4 out_var_TEXCOORD10_centroid;
    float4 out_var_TEXCOORD11_centroid;
    float4 out_var_TEXCOORD4;
    float4 out_var_VS_To_DS_Position;
};

struct main0_patchOut
{
    float4 out_var_PN_POSITION9;
};

struct main0_in
{
    float4 in_var_TEXCOORD10_centroid [[attribute(0)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(1)]];
    float4 in_var_COLOR0 [[attribute(2)]];
    float4 in_var_TEXCOORD0_0 [[attribute(3)]];
    float4 in_var_TEXCOORD4 [[attribute(4)]];
    uint in_var_PRIMITIVE_ID [[attribute(5)]];
    uint in_var_LIGHTMAP_ID [[attribute(6)]];
    float4 in_var_VS_To_DS_Position [[attribute(7)]];
};

kernel void main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], const device type_StructuredBuffer_v4float& View_PrimitiveSceneData [[buffer(1)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    threadgroup FPNTessellationHSToDS temp_var_hullMainRetVal[3];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    spvUnsafeArray<float4, 12> _144 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_TEXCOORD10_centroid, gl_in[1].in_var_TEXCOORD10_centroid, gl_in[2].in_var_TEXCOORD10_centroid, gl_in[3].in_var_TEXCOORD10_centroid, gl_in[4].in_var_TEXCOORD10_centroid, gl_in[5].in_var_TEXCOORD10_centroid, gl_in[6].in_var_TEXCOORD10_centroid, gl_in[7].in_var_TEXCOORD10_centroid, gl_in[8].in_var_TEXCOORD10_centroid, gl_in[9].in_var_TEXCOORD10_centroid, gl_in[10].in_var_TEXCOORD10_centroid, gl_in[11].in_var_TEXCOORD10_centroid });
    spvUnsafeArray<float4, 12> _145 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_TEXCOORD11_centroid, gl_in[1].in_var_TEXCOORD11_centroid, gl_in[2].in_var_TEXCOORD11_centroid, gl_in[3].in_var_TEXCOORD11_centroid, gl_in[4].in_var_TEXCOORD11_centroid, gl_in[5].in_var_TEXCOORD11_centroid, gl_in[6].in_var_TEXCOORD11_centroid, gl_in[7].in_var_TEXCOORD11_centroid, gl_in[8].in_var_TEXCOORD11_centroid, gl_in[9].in_var_TEXCOORD11_centroid, gl_in[10].in_var_TEXCOORD11_centroid, gl_in[11].in_var_TEXCOORD11_centroid });
    spvUnsafeArray<float4, 12> _146 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_COLOR0, gl_in[1].in_var_COLOR0, gl_in[2].in_var_COLOR0, gl_in[3].in_var_COLOR0, gl_in[4].in_var_COLOR0, gl_in[5].in_var_COLOR0, gl_in[6].in_var_COLOR0, gl_in[7].in_var_COLOR0, gl_in[8].in_var_COLOR0, gl_in[9].in_var_COLOR0, gl_in[10].in_var_COLOR0, gl_in[11].in_var_COLOR0 });
    spvUnsafeArray<spvUnsafeArray<float4, 1>, 12> _147 = spvUnsafeArray<spvUnsafeArray<float4, 1>, 12>({ spvUnsafeArray<float4, 1>({ gl_in[0].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[1].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[2].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[3].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[4].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[5].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[6].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[7].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[8].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[9].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[10].in_var_TEXCOORD0_0 }), spvUnsafeArray<float4, 1>({ gl_in[11].in_var_TEXCOORD0_0 }) });
    spvUnsafeArray<float4, 12> _148 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_TEXCOORD4, gl_in[1].in_var_TEXCOORD4, gl_in[2].in_var_TEXCOORD4, gl_in[3].in_var_TEXCOORD4, gl_in[4].in_var_TEXCOORD4, gl_in[5].in_var_TEXCOORD4, gl_in[6].in_var_TEXCOORD4, gl_in[7].in_var_TEXCOORD4, gl_in[8].in_var_TEXCOORD4, gl_in[9].in_var_TEXCOORD4, gl_in[10].in_var_TEXCOORD4, gl_in[11].in_var_TEXCOORD4 });
    spvUnsafeArray<uint, 12> _149 = spvUnsafeArray<uint, 12>({ gl_in[0].in_var_PRIMITIVE_ID, gl_in[1].in_var_PRIMITIVE_ID, gl_in[2].in_var_PRIMITIVE_ID, gl_in[3].in_var_PRIMITIVE_ID, gl_in[4].in_var_PRIMITIVE_ID, gl_in[5].in_var_PRIMITIVE_ID, gl_in[6].in_var_PRIMITIVE_ID, gl_in[7].in_var_PRIMITIVE_ID, gl_in[8].in_var_PRIMITIVE_ID, gl_in[9].in_var_PRIMITIVE_ID, gl_in[10].in_var_PRIMITIVE_ID, gl_in[11].in_var_PRIMITIVE_ID });
    spvUnsafeArray<uint, 12> _150 = spvUnsafeArray<uint, 12>({ gl_in[0].in_var_LIGHTMAP_ID, gl_in[1].in_var_LIGHTMAP_ID, gl_in[2].in_var_LIGHTMAP_ID, gl_in[3].in_var_LIGHTMAP_ID, gl_in[4].in_var_LIGHTMAP_ID, gl_in[5].in_var_LIGHTMAP_ID, gl_in[6].in_var_LIGHTMAP_ID, gl_in[7].in_var_LIGHTMAP_ID, gl_in[8].in_var_LIGHTMAP_ID, gl_in[9].in_var_LIGHTMAP_ID, gl_in[10].in_var_LIGHTMAP_ID, gl_in[11].in_var_LIGHTMAP_ID });
    spvUnsafeArray<float4, 12> _259 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_VS_To_DS_Position, gl_in[1].in_var_VS_To_DS_Position, gl_in[2].in_var_VS_To_DS_Position, gl_in[3].in_var_VS_To_DS_Position, gl_in[4].in_var_VS_To_DS_Position, gl_in[5].in_var_VS_To_DS_Position, gl_in[6].in_var_VS_To_DS_Position, gl_in[7].in_var_VS_To_DS_Position, gl_in[8].in_var_VS_To_DS_Position, gl_in[9].in_var_VS_To_DS_Position, gl_in[10].in_var_VS_To_DS_Position, gl_in[11].in_var_VS_To_DS_Position });
    spvUnsafeArray<FBasePassVSToDS, 12> _284 = spvUnsafeArray<FBasePassVSToDS, 12>({ FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[0], _145[0], _146[0], _147[0], _148[0], _149[0], _150[0] } }, FBasePassInterpolantsVSToDS{ { } }, _259[0] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[1], _145[1], _146[1], _147[1], _148[1], _149[1], _150[1] } }, FBasePassInterpolantsVSToDS{ { } }, _259[1] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[2], _145[2], _146[2], _147[2], _148[2], _149[2], _150[2] } }, FBasePassInterpolantsVSToDS{ { } }, _259[2] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[3], _145[3], _146[3], _147[3], _148[3], _149[3], _150[3] } }, FBasePassInterpolantsVSToDS{ { } }, _259[3] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[4], _145[4], _146[4], _147[4], _148[4], _149[4], _150[4] } }, FBasePassInterpolantsVSToDS{ { } }, _259[4] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[5], _145[5], _146[5], _147[5], _148[5], _149[5], _150[5] } }, FBasePassInterpolantsVSToDS{ { } }, _259[5] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[6], _145[6], _146[6], _147[6], _148[6], _149[6], _150[6] } }, FBasePassInterpolantsVSToDS{ { } }, _259[6] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[7], _145[7], _146[7], _147[7], _148[7], _149[7], _150[7] } }, FBasePassInterpolantsVSToDS{ { } }, _259[7] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[8], _145[8], _146[8], _147[8], _148[8], _149[8], _150[8] } }, FBasePassInterpolantsVSToDS{ { } }, _259[8] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[9], _145[9], _146[9], _147[9], _148[9], _149[9], _150[9] } }, FBasePassInterpolantsVSToDS{ { } }, _259[9] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[10], _145[10], _146[10], _147[10], _148[10], _149[10], _150[10] } }, FBasePassInterpolantsVSToDS{ { } }, _259[10] }, FBasePassVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _144[11], _145[11], _146[11], _147[11], _148[11], _149[11], _150[11] } }, FBasePassInterpolantsVSToDS{ { } }, _259[11] } });
    spvUnsafeArray<FBasePassVSToDS, 12> param_var_I;
    param_var_I = _284;
    float4 _301 = float4(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float3 _310 = View_PrimitiveSceneData._m0[(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.PrimitiveId * 26u) + 22u].xyz * float3x3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz, cross(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz) * float3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.w), param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz);
    uint _313 = (gl_InvocationID < 2u) ? (gl_InvocationID + 1u) : 0u;
    uint _314 = 2u * gl_InvocationID;
    uint _315 = 3u + _314;
    uint _316 = _314 + 4u;
    float4 _328 = float4(param_var_I[_313].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _336 = float4(param_var_I[_315].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _344 = float4(param_var_I[_316].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    spvUnsafeArray<float4, 3> _392 = spvUnsafeArray<float4, 3>({ param_var_I[gl_InvocationID].Position, ((((float4(2.0) * param_var_I[gl_InvocationID].Position) + param_var_I[_313].Position) - (float4(dot(param_var_I[_313].Position - param_var_I[gl_InvocationID].Position, _301)) * _301)) + (((float4(2.0) * param_var_I[_315].Position) + param_var_I[_316].Position) - (float4(dot(param_var_I[_316].Position - param_var_I[_315].Position, _336)) * _336))) * float4(0.16666667163372039794921875), ((((float4(2.0) * param_var_I[_313].Position) + param_var_I[gl_InvocationID].Position) - (float4(dot(param_var_I[gl_InvocationID].Position - param_var_I[_313].Position, _328)) * _328)) + (((float4(2.0) * param_var_I[_316].Position) + param_var_I[_315].Position) - (float4(dot(param_var_I[_315].Position - param_var_I[_316].Position, _344)) * _344))) * float4(0.16666667163372039794921875) });
    gl_out[gl_InvocationID].out_var_TEXCOORD10_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
    gl_out[gl_InvocationID].out_var_TEXCOORD11_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    gl_out[gl_InvocationID].out_var_COLOR0 = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.Color;
    gl_out[gl_InvocationID].out_var_TEXCOORD0 = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TexCoords;
    gl_out[gl_InvocationID].out_var_TEXCOORD4 = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.LightMapCoordinate;
    gl_out[gl_InvocationID].out_var_PRIMITIVE_ID = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.PrimitiveId;
    gl_out[gl_InvocationID].out_var_LIGHTMAP_ID = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.LightmapDataIndex;
    gl_out[gl_InvocationID].out_var_VS_To_DS_Position = param_var_I[gl_InvocationID].Position;
    gl_out[gl_InvocationID].out_var_PN_POSITION = _392;
    gl_out[gl_InvocationID].out_var_PN_DisplacementScales = _310;
    gl_out[gl_InvocationID].out_var_PN_TessellationMultiplier = 1.0;
    gl_out[gl_InvocationID].out_var_PN_WorldDisplacementMultiplier = 1.0;
    temp_var_hullMainRetVal[gl_InvocationID] = FPNTessellationHSToDS{ param_var_I[gl_InvocationID], _392, _310, 1.0, 1.0 };
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    if (gl_InvocationID == 0u)
    {
        float4 _450 = (((((temp_var_hullMainRetVal[0u].WorldPosition[1] + temp_var_hullMainRetVal[0u].WorldPosition[2]) + temp_var_hullMainRetVal[1u].WorldPosition[1]) + temp_var_hullMainRetVal[1u].WorldPosition[2]) + temp_var_hullMainRetVal[2u].WorldPosition[1]) + temp_var_hullMainRetVal[2u].WorldPosition[2]) * float4(0.16666667163372039794921875);
        float4 _463 = _602;
        _463.x = 0.5 * (temp_var_hullMainRetVal[1u].TessellationMultiplier + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _469 = _463;
        _469.y = 0.5 * (temp_var_hullMainRetVal[2u].TessellationMultiplier + temp_var_hullMainRetVal[0u].TessellationMultiplier);
        float4 _474 = _469;
        _474.z = 0.5 * (temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier);
        float4 _481 = _474;
        _481.w = 0.333000004291534423828125 * ((temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier) + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _589;
        for (;;)
        {
            float4 _489 = View.View_ViewToClip * float4(0.0);
            float4 _494 = View.View_TranslatedWorldToClip * float4(temp_var_hullMainRetVal[0u].WorldPosition[0].xyz, 1.0);
            float3 _495 = _494.xyz;
            float3 _496 = _489.xyz;
            float _498 = _494.w;
            float _499 = _489.w;
            float4 _516 = View.View_TranslatedWorldToClip * float4(temp_var_hullMainRetVal[1u].WorldPosition[0].xyz, 1.0);
            float3 _517 = _516.xyz;
            float _519 = _516.w;
            float4 _537 = View.View_TranslatedWorldToClip * float4(temp_var_hullMainRetVal[2u].WorldPosition[0].xyz, 1.0);
            float3 _538 = _537.xyz;
            float _540 = _537.w;
            if (any((((select(int3(0), int3(1), (_495 - _496) < float3(_498 + _499)) + (int3(2) * select(int3(0), int3(1), (_495 + _496) > float3((-_498) - _499)))) | (select(int3(0), int3(1), (_517 - _496) < float3(_519 + _499)) + (int3(2) * select(int3(0), int3(1), (_517 + _496) > float3((-_519) - _499))))) | (select(int3(0), int3(1), (_538 - _496) < float3(_540 + _499)) + (int3(2) * select(int3(0), int3(1), (_538 + _496) > float3((-_540) - _499))))) != int3(3)))
            {
                _589 = float4(0.0);
                break;
            }
            float3 _558 = temp_var_hullMainRetVal[0u].WorldPosition[0].xyz - temp_var_hullMainRetVal[1u].WorldPosition[0].xyz;
            float3 _559 = temp_var_hullMainRetVal[1u].WorldPosition[0].xyz - temp_var_hullMainRetVal[2u].WorldPosition[0].xyz;
            float3 _560 = temp_var_hullMainRetVal[2u].WorldPosition[0].xyz - temp_var_hullMainRetVal[0u].WorldPosition[0].xyz;
            float3 _563 = (float3(0.5) * (temp_var_hullMainRetVal[0u].WorldPosition[0].xyz + temp_var_hullMainRetVal[1u].WorldPosition[0].xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float3 _566 = (float3(0.5) * (temp_var_hullMainRetVal[1u].WorldPosition[0].xyz + temp_var_hullMainRetVal[2u].WorldPosition[0].xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float3 _569 = (float3(0.5) * (temp_var_hullMainRetVal[2u].WorldPosition[0].xyz + temp_var_hullMainRetVal[0u].WorldPosition[0].xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float _573 = sqrt(dot(_559, _559) / dot(_566, _566));
            float _577 = sqrt(dot(_560, _560) / dot(_569, _569));
            float _581 = sqrt(dot(_558, _558) / dot(_563, _563));
            float4 _586 = float4(_573, _577, _581, 1.0);
            _586.w = 0.333000004291534423828125 * ((_573 + _577) + _581);
            _589 = float4(View.View_AdaptiveTessellationFactor) * _586;
            break;
        }
        float4 _591 = fast::clamp(_481 * _589, float4(1.0), float4(15.0));
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0u] = half(_591.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1u] = half(_591.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2u] = half(_591.z);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_591.w);
        patchOut.out_var_PN_POSITION9 = _450 + ((_450 - (((temp_var_hullMainRetVal[2u].WorldPosition[0] + temp_var_hullMainRetVal[1u].WorldPosition[0]) + temp_var_hullMainRetVal[0u].WorldPosition[0]) * float4(0.3333333432674407958984375))) * float4(0.5));
    }
}

