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

struct type_MobileBasePass
{
    float4 MobileBasePass_Fog_ExponentialFogParameters;
    float4 MobileBasePass_Fog_ExponentialFogParameters2;
    float4 MobileBasePass_Fog_ExponentialFogColorParameter;
    float4 MobileBasePass_Fog_ExponentialFogParameters3;
    float4 MobileBasePass_Fog_InscatteringLightDirection;
    float4 MobileBasePass_Fog_DirectionalInscatteringColor;
    float2 MobileBasePass_Fog_SinCosInscatteringColorCubemapRotation;
    float PrePadding_MobileBasePass_Fog_104;
    float PrePadding_MobileBasePass_Fog_108;
    packed_float3 MobileBasePass_Fog_FogInscatteringTextureParameters;
    float MobileBasePass_Fog_ApplyVolumetricFog;
    float PrePadding_MobileBasePass_PlanarReflection_128;
    float PrePadding_MobileBasePass_PlanarReflection_132;
    float PrePadding_MobileBasePass_PlanarReflection_136;
    float PrePadding_MobileBasePass_PlanarReflection_140;
    float PrePadding_MobileBasePass_PlanarReflection_144;
    float PrePadding_MobileBasePass_PlanarReflection_148;
    float PrePadding_MobileBasePass_PlanarReflection_152;
    float PrePadding_MobileBasePass_PlanarReflection_156;
    float4 MobileBasePass_PlanarReflection_ReflectionPlane;
    float4 MobileBasePass_PlanarReflection_PlanarReflectionOrigin;
    float4 MobileBasePass_PlanarReflection_PlanarReflectionXAxis;
    float4 MobileBasePass_PlanarReflection_PlanarReflectionYAxis;
    float3x4 MobileBasePass_PlanarReflection_InverseTransposeMirrorMatrix;
    packed_float3 MobileBasePass_PlanarReflection_PlanarReflectionParameters;
    float PrePadding_MobileBasePass_PlanarReflection_284;
    float2 MobileBasePass_PlanarReflection_PlanarReflectionParameters2;
    float PrePadding_MobileBasePass_PlanarReflection_296;
    float PrePadding_MobileBasePass_PlanarReflection_300;
    float4x4 MobileBasePass_PlanarReflection_ProjectionWithExtraFOV[2];
    float4 MobileBasePass_PlanarReflection_PlanarReflectionScreenScaleBias[2];
    float2 MobileBasePass_PlanarReflection_PlanarReflectionScreenBound;
    uint MobileBasePass_PlanarReflection_bIsStereo;
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
    float Primitive_UseEditorDepthTest;
    float4 Primitive_ObjectOrientation;
    float4 Primitive_NonUniformScale;
    packed_float3 Primitive_LocalObjectBoundsMin;
    float PrePadding_Primitive_380;
    packed_float3 Primitive_LocalObjectBoundsMax;
    uint Primitive_LightingChannelMask;
    uint Primitive_LightmapDataIndex;
    int Primitive_SingleCaptureIndex;
};

struct type_LandscapeParameters
{
    float4 LandscapeParameters_HeightmapUVScaleBias;
    float4 LandscapeParameters_WeightmapUVScaleBias;
    float4 LandscapeParameters_LandscapeLightmapScaleBias;
    float4 LandscapeParameters_SubsectionSizeVertsLayerUVPan;
    float4 LandscapeParameters_SubsectionOffsetParams;
    float4 LandscapeParameters_LightmapSubsectionOffsetParams;
    float4x4 LandscapeParameters_LocalToWorldNoScaling;
};

struct type_Globals
{
    float4 LodBias;
    float4 LodValues;
    float4 SectionLods;
    float4 NeighborSectionLod[4];
};

struct main0_out
{
    float2 out_var_TEXCOORD0 [[user(locn0)]];
    float2 out_var_TEXCOORD1 [[user(locn1)]];
    float4 out_var_TEXCOORD2 [[user(locn2)]];
    float4 out_var_TEXCOORD3 [[user(locn3)]];
    float4 out_var_TEXCOORD8 [[user(locn4)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 in_var_ATTRIBUTE0 [[attribute(0)]];
    float4 in_var_ATTRIBUTE1_0 [[attribute(1)]];
    float4 in_var_ATTRIBUTE1_1 [[attribute(2)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], constant type_MobileBasePass& MobileBasePass [[buffer(1)]], constant type_Primitive& Primitive [[buffer(2)]], constant type_LandscapeParameters& LandscapeParameters [[buffer(3)]], constant type_Globals& _Globals [[buffer(4)]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 2> in_var_ATTRIBUTE1 = {};
    in_var_ATTRIBUTE1[0] = in.in_var_ATTRIBUTE1_0;
    in_var_ATTRIBUTE1[1] = in.in_var_ATTRIBUTE1_1;
    spvUnsafeArray<float4, 1> _97;
    for (int _107 = 0; _107 < 1; )
    {
        _97[_107] = float4(0.0);
        _107++;
        continue;
    }
    float4 _115 = in.in_var_ATTRIBUTE0 * float4(255.0);
    float2 _116 = _115.zw;
    float2 _119 = fract(_116 * float2(0.5)) * float2(2.0);
    float2 _121 = (_116 - _119) * float2(0.0039215688593685626983642578125);
    float2 _122 = _115.xy;
    float2 _126 = _122 * float2(_Globals.LodValues.w);
    float _127 = _126.y;
    float _128 = _126.x;
    float4 _132 = float4(_127, _128, 1.0 - _128, 1.0 - _127) * float4(2.0);
    float4 _186;
    if (_119.y > 0.5)
    {
        float4 _161;
        if (_119.x > 0.5)
        {
            _161 = (_132 * float4(_Globals.SectionLods.w)) + ((float4(1.0) - _132) * _Globals.NeighborSectionLod[3]);
        }
        else
        {
            _161 = (_132 * float4(_Globals.SectionLods.z)) + ((float4(1.0) - _132) * _Globals.NeighborSectionLod[2]);
        }
        _186 = _161;
    }
    else
    {
        float4 _185;
        if (_119.x > 0.5)
        {
            _185 = (_132 * float4(_Globals.SectionLods.y)) + ((float4(1.0) - _132) * _Globals.NeighborSectionLod[1]);
        }
        else
        {
            _185 = (_132 * float4(_Globals.SectionLods.x)) + ((float4(1.0) - _132) * _Globals.NeighborSectionLod[0]);
        }
        _186 = _185;
    }
    float _206;
    if ((_128 + _127) > 1.0)
    {
        float _198;
        if (_128 < _127)
        {
            _198 = _186.w;
        }
        else
        {
            _198 = _186.z;
        }
        _206 = _198;
    }
    else
    {
        float _205;
        if (_128 < _127)
        {
            _205 = _186.y;
        }
        else
        {
            _205 = _186.x;
        }
        _206 = _205;
    }
    float _207 = floor(_206);
    float _220 = _121.x;
    float3 _235 = select(select(select(select(select(float3(0.03125, _121.yy), float3(0.0625, _220, _121.y), bool3(_207 < 5.0)), float3(0.125, in_var_ATTRIBUTE1[1].w, _220), bool3(_207 < 4.0)), float3(0.25, in_var_ATTRIBUTE1[1].zw), bool3(_207 < 3.0)), float3(0.5, in_var_ATTRIBUTE1[1].yz), bool3(_207 < 2.0)), float3(1.0, in_var_ATTRIBUTE1[1].xy), bool3(_207 < 1.0));
    float _236 = _235.x;
    float _245 = (((in_var_ATTRIBUTE1[0].x * 65280.0) + (in_var_ATTRIBUTE1[0].y * 255.0)) - 32768.0) * 0.0078125;
    float _252 = (((in_var_ATTRIBUTE1[0].z * 65280.0) + (in_var_ATTRIBUTE1[0].w * 255.0)) - 32768.0) * 0.0078125;
    float2 _257 = floor(_122 * float2(_236));
    float2 _271 = float2((LandscapeParameters.LandscapeParameters_SubsectionSizeVertsLayerUVPan.x * _236) - 1.0, fast::max((LandscapeParameters.LandscapeParameters_SubsectionSizeVertsLayerUVPan.x * 0.5) * _236, 2.0) - 1.0) * float2(LandscapeParameters.LandscapeParameters_SubsectionSizeVertsLayerUVPan.y);
    float3 _287 = mix(float3(_257 / float2(_271.x), mix(_245, _252, _235.y)), float3(floor(_257 * float2(0.5)) / float2(_271.y), mix(_245, _252, _235.z)), float3(_206 - _207));
    float2 _288 = _119.xy;
    float2 _292 = _288 * LandscapeParameters.LandscapeParameters_SubsectionOffsetParams.ww;
    float3 _296 = _287 + float3(_292, 0.0);
    float4 _322 = float4((((Primitive.Primitive_LocalToWorld[0u].xyz * _296.xxx) + (Primitive.Primitive_LocalToWorld[1u].xyz * _296.yyy)) + (Primitive.Primitive_LocalToWorld[2u].xyz * _296.zzz)) + (Primitive.Primitive_LocalToWorld[3u].xyz + float3(View.View_PreViewTranslation)), 1.0);
    float2 _323 = _287.xy;
    float4 _338 = float4(_322.x, _322.y, _322.z, _322.w);
    float4 _339 = View.View_TranslatedWorldToClip * _338;
    float3 _341 = _322.xyz - float3(View.View_TranslatedWorldCameraOrigin);
    float _345 = dot(_341, _341);
    float _346 = rsqrt(_345);
    float _347 = _345 * _346;
    float _354 = _341.z;
    float _357 = fast::max(0.0, MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters.w);
    float _393;
    float _394;
    float _395;
    float _396;
    if (_357 > 0.0)
    {
        float _361 = _357 * _346;
        float _362 = _361 * _354;
        float _365 = View.View_WorldCameraOrigin[2] + _362;
        _393 = (1.0 - _361) * _347;
        _394 = MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters2.z * exp2(-fast::max(-127.0, MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters2.y * (_365 - MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters2.w)));
        _395 = MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters3.x * exp2(-fast::max(-127.0, MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters.y * (_365 - MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters3.y)));
        _396 = _354 - _362;
    }
    else
    {
        _393 = _347;
        _394 = MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters2.x;
        _395 = MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters.x;
        _396 = _354;
    }
    float _400 = fast::max(-127.0, MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters.y * _396);
    float _405 = log(2.0);
    float _407 = 0.5 * (_405 * _405);
    float _417 = fast::max(-127.0, MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters2.y * _396);
    float _428 = (_395 * ((abs(_400) > 0.00999999977648258209228515625) ? ((1.0 - exp2(-_400)) / _400) : (_405 - (_407 * _400)))) + (_394 * ((abs(_417) > 0.00999999977648258209228515625) ? ((1.0 - exp2(-_417)) / _417) : (_405 - (_407 * _417))));
    float3 _459;
    if (MobileBasePass.MobileBasePass_Fog_InscatteringLightDirection.w >= 0.0)
    {
        _459 = (MobileBasePass.MobileBasePass_Fog_DirectionalInscatteringColor.xyz * float3(pow(fast::clamp(dot(_341 * float3(_346), MobileBasePass.MobileBasePass_Fog_InscatteringLightDirection.xyz), 0.0, 1.0), MobileBasePass.MobileBasePass_Fog_DirectionalInscatteringColor.w))) * float3(1.0 - fast::clamp(exp2(-(_428 * fast::max(_393 - MobileBasePass.MobileBasePass_Fog_InscatteringLightDirection.w, 0.0))), 0.0, 1.0));
    }
    else
    {
        _459 = float3(0.0);
    }
    bool _468 = (MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters3.w > 0.0) && (_347 > MobileBasePass.MobileBasePass_Fog_ExponentialFogParameters3.w);
    float _471 = _468 ? 1.0 : fast::max(fast::clamp(exp2(-(_428 * _393)), 0.0, 1.0), MobileBasePass.MobileBasePass_Fog_ExponentialFogColorParameter.w);
    _97[0] = float4((MobileBasePass.MobileBasePass_Fog_ExponentialFogColorParameter.xyz * float3(1.0 - _471)) + select(_459, float3(0.0), bool3(_468)), _471);
    float4 _482 = _338;
    _482.w = _339.w;
    out.out_var_TEXCOORD0 = ((_323 + LandscapeParameters.LandscapeParameters_SubsectionSizeVertsLayerUVPan.zw) + _292).xy;
    out.out_var_TEXCOORD1 = ((_323 * LandscapeParameters.LandscapeParameters_WeightmapUVScaleBias.xy) + LandscapeParameters.LandscapeParameters_WeightmapUVScaleBias.zw) + (_288 * LandscapeParameters.LandscapeParameters_SubsectionOffsetParams.zz);
    out.out_var_TEXCOORD2 = float4(float4(0.0).x, float4(0.0).y, _97[0].x, _97[0].y);
    out.out_var_TEXCOORD3 = float4(float4(0.0).x, float4(0.0).y, _97[0].z, _97[0].w);
    out.out_var_TEXCOORD8 = _482;
    out.gl_Position = _339;
    return out;
}

