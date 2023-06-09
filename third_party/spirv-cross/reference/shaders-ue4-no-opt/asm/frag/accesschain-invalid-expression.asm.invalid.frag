#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

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

struct type_MobileDirectionalLight
{
    float4 MobileDirectionalLight_DirectionalLightColor;
    float4 MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition;
    float4 MobileDirectionalLight_DirectionalLightShadowSize;
    float4 MobileDirectionalLight_DirectionalLightDistanceFadeMAD;
    float4 MobileDirectionalLight_DirectionalLightShadowDistances;
    float4x4 MobileDirectionalLight_DirectionalLightScreenToShadow[4];
};

struct type_Globals
{
    int NumDynamicPointLights;
    float4 LightPositionAndInvRadius[4];
    float4 LightColorAndFalloffExponent[4];
    float4 MobileReflectionParams;
};

constant float3 _136 = {};
constant float4 _137 = {};
constant float _138 = {};
constant float3 _139 = {};

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct main0_in
{
    float2 in_var_TEXCOORD0 [[user(locn0)]];
    float4 in_var_TEXCOORD7 [[user(locn1)]];
    float4 in_var_TEXCOORD8 [[user(locn2)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], constant type_MobileDirectionalLight& MobileDirectionalLight [[buffer(1)]], constant type_Globals& _Globals [[buffer(2)]], texture2d<float> MobileDirectionalLight_DirectionalLightShadowTexture [[texture(0)]], texture2d<float> Material_Texture2D_0 [[texture(1)]], texture2d<float> Material_Texture2D_1 [[texture(2)]], texturecube<float> ReflectionCubemap [[texture(3)]], sampler MobileDirectionalLight_DirectionalLightShadowSampler [[sampler(0)]], sampler Material_Texture2D_0Sampler [[sampler(1)]], sampler Material_Texture2D_1Sampler [[sampler(2)]], sampler ReflectionCubemapSampler [[sampler(3)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float4 _177 = float4((((gl_FragCoord.xy - View.View_ViewRectMin.xy) * View.View_ViewSizeAndInvSize.zw) - float2(0.5)) * float2(2.0, -2.0), _138, 1.0) * float4(gl_FragCoord.w);
    float3 _179 = in.in_var_TEXCOORD8.xyz - float3(View.View_PreViewTranslation);
    float3 _181 = fast::normalize(-in.in_var_TEXCOORD8.xyz);
    float4 _187 = Material_Texture2D_0.sample(Material_Texture2D_0Sampler, (in.in_var_TEXCOORD0 * float2(10.0)));
    float2 _190 = (_187.xy * float2(2.0)) - float2(1.0);
    float3 _206 = fast::normalize(float3x3(float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, 1.0)) * (((float4(_190, sqrt(fast::clamp(1.0 - dot(_190, _190), 0.0, 1.0)), 1.0).xyz * float3(0.300000011920928955078125, 0.300000011920928955078125, 1.0)) * float3(View.View_NormalOverrideParameter.w)) + View.View_NormalOverrideParameter.xyz));
    float _208 = dot(_206, _181);
    float4 _217 = Material_Texture2D_1.sample(Material_Texture2D_1Sampler, (in.in_var_TEXCOORD0 * float2(20.0)));
    float _219 = mix(0.4000000059604644775390625, 1.0, _217.x);
    float4 _223 = Material_Texture2D_1.sample(Material_Texture2D_1Sampler, (in.in_var_TEXCOORD0 * float2(5.0)));
    float _224 = _177.w;
    float _228 = fast::min(fast::max((_224 - 24.0) * 0.000666666659526526927947998046875, 0.0), 1.0);
    float _229 = _223.y;
    float4 _233 = Material_Texture2D_1.sample(Material_Texture2D_1Sampler, (in.in_var_TEXCOORD0 * float2(0.5)));
    float _235 = _233.y;
    float _253 = fast::clamp((fast::min(fast::max(mix(0.0, 0.5, _235) + mix(mix(0.699999988079071044921875, 1.0, _229), 1.0, _228), 0.0), 1.0) * View.View_RoughnessOverrideParameter.y) + View.View_RoughnessOverrideParameter.x, 0.119999997317790985107421875, 1.0);
    float2 _257 = (float2(_253) * float2(-1.0, -0.0274999998509883880615234375)) + float2(1.0, 0.0425000004470348358154296875);
    float _258 = _257.x;
    float3 _270 = (fast::clamp(float3(mix(_219, 1.0 - _219, mix(_229, 1.0, _228)) * (mix(0.2949999868869781494140625, 0.660000026226043701171875, mix(_235 + mix(_229, 0.0, _228), 0.5, 0.5)) * 0.5)), float3(0.0), float3(1.0)) * float3(View.View_DiffuseOverrideParameter.w)) + View.View_DiffuseOverrideParameter.xyz;
    float3 _275 = float3(((fast::min(_258 * _258, exp2((-9.27999973297119140625) * fast::max(_208, 0.0))) * _258) + _257.y) * View.View_SpecularOverrideParameter.w) + View.View_SpecularOverrideParameter.xyz;
    float _276 = _275.x;
    float4 _303;
    int _286 = 0;
    for (;;)
    {
        if (_286 < 2)
        {
            if (_224 < MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowDistances[uint(_286)])
            {
                _303 = MobileDirectionalLight.MobileDirectionalLight_DirectionalLightScreenToShadow[_286] * float4(_177.xy, _224, 1.0);
                break;
            }
            _286++;
            continue;
        }
        else
        {
            _303 = float4(0.0);
            break;
        }
    }
    float _423;
    if (_303.z > 0.0)
    {
        float2 _311 = _303.xy * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.xy;
        float2 _312 = fract(_311);
        float2 _313 = floor(_311);
        float3 _320;
        _320.x = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(-0.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        _320.y = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(0.5, -0.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        _320.z = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(1.5, -0.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        float3 _335 = float3(MobileDirectionalLight.MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition.w);
        float3 _337 = float3((fast::min(_303.z, 0.999989986419677734375) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition.w) - 1.0);
        float3 _339 = fast::clamp((_320 * _335) - _337, float3(0.0), float3(1.0));
        float3 _345;
        _345.x = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(-0.5, 0.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        _345.y = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(0.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        _345.z = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(1.5, 0.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        float3 _360 = fast::clamp((_345 * _335) - _337, float3(0.0), float3(1.0));
        float3 _366;
        _366.x = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(-0.5, 1.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        _366.y = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(0.5, 1.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        _366.z = MobileDirectionalLight_DirectionalLightShadowTexture.sample(MobileDirectionalLight_DirectionalLightShadowSampler, ((_313 + float2(1.5)) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightShadowSize.zw), level(0.0)).x;
        float3 _381 = fast::clamp((_366 * _335) - _337, float3(0.0), float3(1.0));
        float _383 = _312.x;
        float _384 = 1.0 - _383;
        float3 _399;
        _399.x = ((_339.x * _384) + _339.y) + (_339.z * _383);
        _399.y = ((_360.x * _384) + _360.y) + (_360.z * _383);
        _399.z = ((_381.x * _384) + _381.y) + (_381.z * _383);
        float _408 = _312.y;
        float _420 = fast::clamp((_224 * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightDistanceFadeMAD.x) + MobileDirectionalLight.MobileDirectionalLight_DirectionalLightDistanceFadeMAD.y, 0.0, 1.0);
        _423 = mix(fast::clamp(0.25 * dot(_399, float3(1.0 - _408, 1.0, _408)), 0.0, 1.0), 1.0, _420 * _420);
    }
    else
    {
        _423 = 1.0;
    }
    float3 _429 = fast::normalize(_181 + MobileDirectionalLight.MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition.xyz);
    float _439 = (_253 * 0.25) + 0.25;
    float3 _440 = cross(_206, _429);
    float _442 = _253 * _253;
    float _443 = fast::max(0.0, dot(_206, _429)) * _442;
    float _446 = _442 / (dot(_440, _440) + (_443 * _443));
    bool _458 = float(_Globals.MobileReflectionParams.w > 0.0) != 0.0;
    float4 _468 = ReflectionCubemap.sample(ReflectionCubemapSampler, ((-_181) + ((_206 * float3(_208)) * float3(2.0))), level(((_458 ? _Globals.MobileReflectionParams.w : View.View_ReflectionCubemapMaxMip) - 1.0) - (1.0 - (1.2000000476837158203125 * log2(_253)))));
    float3 _481;
    if (_458)
    {
        _481 = _468.xyz * View.View_SkyLightColor.xyz;
    }
    else
    {
        float3 _476 = _468.xyz * float3(_468.w * 16.0);
        _481 = _476 * _476;
    }
    float3 _484 = float3(_276);
    float3 _488;
    _488 = ((float3(_423 * fast::max(0.0, dot(_206, MobileDirectionalLight.MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition.xyz))) * MobileDirectionalLight.MobileDirectionalLight_DirectionalLightColor.xyz) * (_270 + float3(_276 * (_439 * fast::min(_446 * _446, 65504.0))))) + ((_481 * float3(fast::clamp(1.0, 0.0, 1.0))) * _484);
    float3 _507;
    float _509;
    float _511;
    float _537;
    int _491 = 0;
    for (;;)
    {
        if (_491 < _Globals.NumDynamicPointLights)
        {
            float3 _501 = _Globals.LightPositionAndInvRadius[_491].xyz - _179;
            float _502 = dot(_501, _501);
            float3 _505 = _501 * float3(rsqrt(_502));
            _507 = fast::normalize(_181 + _505);
            _509 = fast::max(0.0, dot(_206, _505));
            _511 = fast::max(0.0, dot(_206, _507));
            if (_Globals.LightColorAndFalloffExponent[_491].w == 0.0)
            {
                float _531 = _502 * (_Globals.LightPositionAndInvRadius[_491].w * _Globals.LightPositionAndInvRadius[_491].w);
                float _534 = fast::clamp(1.0 - (_531 * _531), 0.0, 1.0);
                _537 = (1.0 / (_502 + 1.0)) * (_534 * _534);
            }
            else
            {
                float3 _521 = _501 * float3(_Globals.LightPositionAndInvRadius[_491].w);
                _537 = pow(1.0 - fast::clamp(dot(_521, _521), 0.0, 1.0), _Globals.LightColorAndFalloffExponent[_491].w);
            }
            float3 _544 = cross(_206, _507);
            float _546 = _511 * _442;
            float _549 = _442 / (dot(_544, _544) + (_546 * _546));
            _488 += fast::min(float3(65000.0), ((float3(_537 * _509) * _Globals.LightColorAndFalloffExponent[_491].xyz) * float3(0.3183098733425140380859375)) * (_270 + float3(_276 * (_439 * fast::min(_549 * _549, 65504.0)))));
            _491++;
            continue;
        }
        else
        {
            break;
        }
    }
    float3 _567 = (mix(_488 + fast::max(float3(0.0), float3(0.0)), _270 + _484, float3(View.View_UnlitViewmodeMask)) * float3(in.in_var_TEXCOORD7.w)) + in.in_var_TEXCOORD7.xyz;
    float4 _568 = float4(_567.x, _567.y, _567.z, _137.w);
    _568.w = fast::min(in.in_var_TEXCOORD8.w, 65500.0);
    out.out_var_SV_Target0 = _568;
    return out;
}

