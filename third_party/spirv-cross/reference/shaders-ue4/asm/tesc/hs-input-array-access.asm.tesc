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
};

struct FVertexFactoryInterpolantsVSToDS
{
    FVertexFactoryInterpolantsVSToPS InterpolantsVSToPS;
};

struct FHitProxyVSToDS
{
    FVertexFactoryInterpolantsVSToDS FactoryInterpolants;
    float4 Position;
    uint VertexID;
};

struct FHullShaderConstantDominantVertexData
{
    float2 UV;
    float4 Normal;
    float3 Tangent;
};

struct FHullShaderConstantDominantEdgeData
{
    float2 UV0;
    float2 UV1;
    float4 Normal0;
    float4 Normal1;
    float3 Tangent0;
    float3 Tangent1;
};

struct FPNTessellationHSToDS
{
    FHitProxyVSToDS PassSpecificData;
    spvUnsafeArray<float4, 3> WorldPosition;
    float3 DisplacementScale;
    float TessellationMultiplier;
    float WorldDisplacementMultiplier;
    FHullShaderConstantDominantVertexData DominantVertex;
    FHullShaderConstantDominantEdgeData DominantEdge;
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

struct type_Primitive
{
    float4x4 Primitive_LocalToWorld;
    float4 Primitive_InvNonUniformScaleAndDeterminantSign;
    float4 Primitive_ObjectWorldPositionAndRadius;
    float4x4 Primitive_WorldToLocal;
    float4x4 Primitive_PreviousLocalToWorld;
    float4x4 Primitive_PreviousWorldToLocal;
    packed_float3 Primitive_ActorWorldPosition;
    float Primitive_UseSingleSampleShadowFromStationaryLights;
    packed_float3 Primitive_ObjectBounds;
    float Primitive_LpvBiasMultiplier;
    float Primitive_DecalReceiverMask;
    float Primitive_PerObjectGBufferData;
    float Primitive_UseVolumetricLightmapShadowFromStationaryLights;
    float Primitive_DrawsVelocity;
    float4 Primitive_ObjectOrientation;
    float4 Primitive_NonUniformScale;
    packed_float3 Primitive_LocalObjectBoundsMin;
    uint Primitive_LightingChannelMask;
    packed_float3 Primitive_LocalObjectBoundsMax;
    uint Primitive_LightmapDataIndex;
    packed_float3 Primitive_PreSkinnedLocalBounds;
    int Primitive_SingleCaptureIndex;
    uint Primitive_OutputVelocity;
    uint PrePadding_Primitive_420;
    uint PrePadding_Primitive_424;
    uint PrePadding_Primitive_428;
    float4 Primitive_CustomPrimitiveData[4];
};

constant float4 _140 = {};

struct main0_out
{
    float3 out_var_PN_DisplacementScales;
    float2 out_var_PN_DominantEdge;
    float2 out_var_PN_DominantEdge1;
    float4 out_var_PN_DominantEdge2;
    float4 out_var_PN_DominantEdge3;
    float3 out_var_PN_DominantEdge4;
    float3 out_var_PN_DominantEdge5;
    float2 out_var_PN_DominantVertex;
    float4 out_var_PN_DominantVertex1;
    float3 out_var_PN_DominantVertex2;
    spvUnsafeArray<float4, 3> out_var_PN_POSITION;
    float out_var_PN_TessellationMultiplier;
    float out_var_PN_WorldDisplacementMultiplier;
    float4 out_var_TEXCOORD10_centroid;
    float4 out_var_TEXCOORD11_centroid;
    float4 out_var_VS_To_DS_Position;
    uint out_var_VS_To_DS_VertexID;
};

struct main0_patchOut
{
    float4 out_var_PN_POSITION9;
};

struct main0_in
{
    float4 in_var_TEXCOORD10_centroid [[attribute(0)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(1)]];
    float4 in_var_VS_To_DS_Position [[attribute(2)]];
    uint in_var_VS_To_DS_VertexID [[attribute(3)]];
};

kernel void main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], constant type_Primitive& Primitive [[buffer(1)]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    threadgroup FPNTessellationHSToDS temp_var_hullMainRetVal[3];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 3];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 3)
        return;
    spvUnsafeArray<float4, 12> _142 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_TEXCOORD10_centroid, gl_in[1].in_var_TEXCOORD10_centroid, gl_in[2].in_var_TEXCOORD10_centroid, gl_in[3].in_var_TEXCOORD10_centroid, gl_in[4].in_var_TEXCOORD10_centroid, gl_in[5].in_var_TEXCOORD10_centroid, gl_in[6].in_var_TEXCOORD10_centroid, gl_in[7].in_var_TEXCOORD10_centroid, gl_in[8].in_var_TEXCOORD10_centroid, gl_in[9].in_var_TEXCOORD10_centroid, gl_in[10].in_var_TEXCOORD10_centroid, gl_in[11].in_var_TEXCOORD10_centroid });
    spvUnsafeArray<float4, 12> _143 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_TEXCOORD11_centroid, gl_in[1].in_var_TEXCOORD11_centroid, gl_in[2].in_var_TEXCOORD11_centroid, gl_in[3].in_var_TEXCOORD11_centroid, gl_in[4].in_var_TEXCOORD11_centroid, gl_in[5].in_var_TEXCOORD11_centroid, gl_in[6].in_var_TEXCOORD11_centroid, gl_in[7].in_var_TEXCOORD11_centroid, gl_in[8].in_var_TEXCOORD11_centroid, gl_in[9].in_var_TEXCOORD11_centroid, gl_in[10].in_var_TEXCOORD11_centroid, gl_in[11].in_var_TEXCOORD11_centroid });
    spvUnsafeArray<float4, 12> _192 = spvUnsafeArray<float4, 12>({ gl_in[0].in_var_VS_To_DS_Position, gl_in[1].in_var_VS_To_DS_Position, gl_in[2].in_var_VS_To_DS_Position, gl_in[3].in_var_VS_To_DS_Position, gl_in[4].in_var_VS_To_DS_Position, gl_in[5].in_var_VS_To_DS_Position, gl_in[6].in_var_VS_To_DS_Position, gl_in[7].in_var_VS_To_DS_Position, gl_in[8].in_var_VS_To_DS_Position, gl_in[9].in_var_VS_To_DS_Position, gl_in[10].in_var_VS_To_DS_Position, gl_in[11].in_var_VS_To_DS_Position });
    spvUnsafeArray<uint, 12> _193 = spvUnsafeArray<uint, 12>({ gl_in[0].in_var_VS_To_DS_VertexID, gl_in[1].in_var_VS_To_DS_VertexID, gl_in[2].in_var_VS_To_DS_VertexID, gl_in[3].in_var_VS_To_DS_VertexID, gl_in[4].in_var_VS_To_DS_VertexID, gl_in[5].in_var_VS_To_DS_VertexID, gl_in[6].in_var_VS_To_DS_VertexID, gl_in[7].in_var_VS_To_DS_VertexID, gl_in[8].in_var_VS_To_DS_VertexID, gl_in[9].in_var_VS_To_DS_VertexID, gl_in[10].in_var_VS_To_DS_VertexID, gl_in[11].in_var_VS_To_DS_VertexID });
    spvUnsafeArray<FHitProxyVSToDS, 12> _230 = spvUnsafeArray<FHitProxyVSToDS, 12>({ FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[0], _143[0] } }, _192[0], _193[0] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[1], _143[1] } }, _192[1], _193[1] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[2], _143[2] } }, _192[2], _193[2] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[3], _143[3] } }, _192[3], _193[3] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[4], _143[4] } }, _192[4], _193[4] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[5], _143[5] } }, _192[5], _193[5] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[6], _143[6] } }, _192[6], _193[6] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[7], _143[7] } }, _192[7], _193[7] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[8], _143[8] } }, _192[8], _193[8] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[9], _143[9] } }, _192[9], _193[9] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[10], _143[10] } }, _192[10], _193[10] }, FHitProxyVSToDS{ FVertexFactoryInterpolantsVSToDS{ FVertexFactoryInterpolantsVSToPS{ _142[11], _143[11] } }, _192[11], _193[11] } });
    spvUnsafeArray<FHitProxyVSToDS, 12> param_var_I;
    param_var_I = _230;
    float4 _247 = float4(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float3 _251 = Primitive.Primitive_NonUniformScale.xyz * float3x3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz, cross(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz) * float3(param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.w), param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz);
    uint _254 = (gl_InvocationID < 2u) ? (gl_InvocationID + 1u) : 0u;
    uint _255 = 2u * gl_InvocationID;
    uint _256 = 3u + _255;
    uint _257 = _255 + 4u;
    uint _264 = (_254 < 2u) ? (_254 + 1u) : 0u;
    uint _265 = 2u * _254;
    uint _266 = 3u + _265;
    uint _267 = _265 + 4u;
    float4 _279 = float4(param_var_I[9u + gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _315;
    float4 _316;
    float4 _317;
    float4 _318;
    if ((param_var_I[_266].VertexID < param_var_I[_254].VertexID) || ((param_var_I[_266].VertexID == param_var_I[_254].VertexID) && (param_var_I[_267].VertexID < param_var_I[_264].VertexID)))
    {
        _315 = param_var_I[_267].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
        _316 = param_var_I[_267].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
        _317 = param_var_I[_266].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
        _318 = param_var_I[_266].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    }
    else
    {
        _315 = param_var_I[_264].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
        _316 = param_var_I[_264].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
        _317 = param_var_I[_254].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
        _318 = param_var_I[_254].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    }
    float4 _324 = float4(_318.xyz, 0.0);
    float4 _328 = float4(_316.xyz, 0.0);
    float4 _336 = float4(param_var_I[_254].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _344 = float4(param_var_I[_256].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    float4 _352 = float4(param_var_I[_257].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2.xyz, 0.0);
    spvUnsafeArray<float4, 3> _402 = spvUnsafeArray<float4, 3>({ param_var_I[gl_InvocationID].Position, (((((float4(2.0) * param_var_I[gl_InvocationID].Position) + param_var_I[_254].Position) - (float4(dot(param_var_I[_254].Position - param_var_I[gl_InvocationID].Position, _247)) * _247)) * float4(0.3333333432674407958984375)) + ((((float4(2.0) * param_var_I[_256].Position) + param_var_I[_257].Position) - (float4(dot(param_var_I[_257].Position - param_var_I[_256].Position, _344)) * _344)) * float4(0.3333333432674407958984375))) * float4(0.5), (((((float4(2.0) * param_var_I[_254].Position) + param_var_I[gl_InvocationID].Position) - (float4(dot(param_var_I[gl_InvocationID].Position - param_var_I[_254].Position, _336)) * _336)) * float4(0.3333333432674407958984375)) + ((((float4(2.0) * param_var_I[_257].Position) + param_var_I[_256].Position) - (float4(dot(param_var_I[_256].Position - param_var_I[_257].Position, _352)) * _352)) * float4(0.3333333432674407958984375))) * float4(0.5) });
    gl_out[gl_InvocationID].out_var_TEXCOORD10_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0;
    gl_out[gl_InvocationID].out_var_TEXCOORD11_centroid = param_var_I[gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld2;
    gl_out[gl_InvocationID].out_var_VS_To_DS_Position = param_var_I[gl_InvocationID].Position;
    gl_out[gl_InvocationID].out_var_VS_To_DS_VertexID = param_var_I[gl_InvocationID].VertexID;
    gl_out[gl_InvocationID].out_var_PN_POSITION = _402;
    gl_out[gl_InvocationID].out_var_PN_DisplacementScales = _251;
    gl_out[gl_InvocationID].out_var_PN_TessellationMultiplier = 1.0;
    gl_out[gl_InvocationID].out_var_PN_WorldDisplacementMultiplier = 1.0;
    gl_out[gl_InvocationID].out_var_PN_DominantVertex = float2(0.0);
    gl_out[gl_InvocationID].out_var_PN_DominantVertex1 = _279;
    gl_out[gl_InvocationID].out_var_PN_DominantVertex2 = param_var_I[9u + gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz;
    gl_out[gl_InvocationID].out_var_PN_DominantEdge = float2(0.0);
    gl_out[gl_InvocationID].out_var_PN_DominantEdge1 = float2(0.0);
    gl_out[gl_InvocationID].out_var_PN_DominantEdge2 = _324;
    gl_out[gl_InvocationID].out_var_PN_DominantEdge3 = _328;
    gl_out[gl_InvocationID].out_var_PN_DominantEdge4 = _317.xyz;
    gl_out[gl_InvocationID].out_var_PN_DominantEdge5 = _315.xyz;
    temp_var_hullMainRetVal[gl_InvocationID] = FPNTessellationHSToDS{ param_var_I[gl_InvocationID], _402, _251, 1.0, 1.0, FHullShaderConstantDominantVertexData{ float2(0.0), _279, param_var_I[9u + gl_InvocationID].FactoryInterpolants.InterpolantsVSToPS.TangentToWorld0.xyz }, FHullShaderConstantDominantEdgeData{ float2(0.0), float2(0.0), _324, _328, _317.xyz, _315.xyz } };
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    if (gl_InvocationID == 0u)
    {
        float4 _461 = (((((temp_var_hullMainRetVal[0u].WorldPosition[1] + temp_var_hullMainRetVal[0u].WorldPosition[2]) + temp_var_hullMainRetVal[1u].WorldPosition[1]) + temp_var_hullMainRetVal[1u].WorldPosition[2]) + temp_var_hullMainRetVal[2u].WorldPosition[1]) + temp_var_hullMainRetVal[2u].WorldPosition[2]) * float4(0.16666667163372039794921875);
        float4 _474 = _140;
        _474.x = 0.5 * (temp_var_hullMainRetVal[1u].TessellationMultiplier + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _480 = _474;
        _480.y = 0.5 * (temp_var_hullMainRetVal[2u].TessellationMultiplier + temp_var_hullMainRetVal[0u].TessellationMultiplier);
        float4 _485 = _480;
        _485.z = 0.5 * (temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier);
        float4 _492 = _485;
        _492.w = 0.333000004291534423828125 * ((temp_var_hullMainRetVal[0u].TessellationMultiplier + temp_var_hullMainRetVal[1u].TessellationMultiplier) + temp_var_hullMainRetVal[2u].TessellationMultiplier);
        float4 _600;
        for (;;)
        {
            float4 _500 = View.View_ViewToClip * float4(0.0);
            float4 _505 = View.View_TranslatedWorldToClip * float4(temp_var_hullMainRetVal[0u].WorldPosition[0].xyz, 1.0);
            float3 _506 = _505.xyz;
            float3 _507 = _500.xyz;
            float _509 = _505.w;
            float _510 = _500.w;
            float4 _527 = View.View_TranslatedWorldToClip * float4(temp_var_hullMainRetVal[1u].WorldPosition[0].xyz, 1.0);
            float3 _528 = _527.xyz;
            float _530 = _527.w;
            float4 _548 = View.View_TranslatedWorldToClip * float4(temp_var_hullMainRetVal[2u].WorldPosition[0].xyz, 1.0);
            float3 _549 = _548.xyz;
            float _551 = _548.w;
            if (any((((select(int3(0), int3(1), (_506 - _507) < float3(_509 + _510)) + (int3(2) * select(int3(0), int3(1), (_506 + _507) > float3((-_509) - _510)))) | (select(int3(0), int3(1), (_528 - _507) < float3(_530 + _510)) + (int3(2) * select(int3(0), int3(1), (_528 + _507) > float3((-_530) - _510))))) | (select(int3(0), int3(1), (_549 - _507) < float3(_551 + _510)) + (int3(2) * select(int3(0), int3(1), (_549 + _507) > float3((-_551) - _510))))) != int3(3)))
            {
                _600 = float4(0.0);
                break;
            }
            float3 _569 = temp_var_hullMainRetVal[0u].WorldPosition[0].xyz - temp_var_hullMainRetVal[1u].WorldPosition[0].xyz;
            float3 _570 = temp_var_hullMainRetVal[1u].WorldPosition[0].xyz - temp_var_hullMainRetVal[2u].WorldPosition[0].xyz;
            float3 _571 = temp_var_hullMainRetVal[2u].WorldPosition[0].xyz - temp_var_hullMainRetVal[0u].WorldPosition[0].xyz;
            float3 _574 = (float3(0.5) * (temp_var_hullMainRetVal[0u].WorldPosition[0].xyz + temp_var_hullMainRetVal[1u].WorldPosition[0].xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float3 _577 = (float3(0.5) * (temp_var_hullMainRetVal[1u].WorldPosition[0].xyz + temp_var_hullMainRetVal[2u].WorldPosition[0].xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float3 _580 = (float3(0.5) * (temp_var_hullMainRetVal[2u].WorldPosition[0].xyz + temp_var_hullMainRetVal[0u].WorldPosition[0].xyz)) - float3(View.View_TranslatedWorldCameraOrigin);
            float _584 = sqrt(dot(_570, _570) / dot(_577, _577));
            float _588 = sqrt(dot(_571, _571) / dot(_580, _580));
            float _592 = sqrt(dot(_569, _569) / dot(_574, _574));
            float4 _597 = float4(_584, _588, _592, 1.0);
            _597.w = 0.333000004291534423828125 * ((_584 + _588) + _592);
            _600 = float4(View.View_AdaptiveTessellationFactor) * _597;
            break;
        }
        float4 _602 = fast::clamp(_492 * _600, float4(1.0), float4(15.0));
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0u] = half(_602.x);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1u] = half(_602.y);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2u] = half(_602.z);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_602.w);
        patchOut.out_var_PN_POSITION9 = _461 + ((_461 - (((temp_var_hullMainRetVal[2u].WorldPosition[0] + temp_var_hullMainRetVal[1u].WorldPosition[0]) + temp_var_hullMainRetVal[0u].WorldPosition[0]) * float4(0.3333333432674407958984375))) * float4(0.5));
    }
}

