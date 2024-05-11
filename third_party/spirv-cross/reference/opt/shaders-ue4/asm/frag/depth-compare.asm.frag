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

struct type_Globals
{
    float3 SoftTransitionScale;
    float4x4 ShadowViewProjectionMatrices[6];
    float InvShadowmapResolution;
    float ShadowFadeFraction;
    float ShadowSharpen;
    float4 LightPositionAndInvRadius;
    float2 ProjectionDepthBiasParameters;
    float4 PointLightDepthBiasAndProjParameters;
};

constant float4 _471 = {};

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

fragment main0_out main0(constant type_View& View [[buffer(0)]], constant type_Globals& _Globals [[buffer(1)]], texture2d<float> SceneTexturesStruct_SceneDepthTexture [[texture(0)]], texture2d<float> SceneTexturesStruct_GBufferATexture [[texture(1)]], texture2d<float> SceneTexturesStruct_GBufferBTexture [[texture(2)]], texture2d<float> SceneTexturesStruct_GBufferDTexture [[texture(3)]], depthcube<float> ShadowDepthCubeTexture [[texture(4)]], texture2d<float> SSProfilesTexture [[texture(5)]], sampler SceneTexturesStruct_SceneDepthTextureSampler [[sampler(0)]], sampler SceneTexturesStruct_GBufferATextureSampler [[sampler(1)]], sampler SceneTexturesStruct_GBufferBTextureSampler [[sampler(2)]], sampler SceneTexturesStruct_GBufferDTextureSampler [[sampler(3)]], sampler ShadowDepthTextureSampler [[sampler(4)]], sampler ShadowDepthCubeTextureSampler [[sampler(5)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    float2 _114 = gl_FragCoord.xy * View.View_BufferSizeAndInvSize.zw;
    float4 _118 = SceneTexturesStruct_SceneDepthTexture.sample(SceneTexturesStruct_SceneDepthTextureSampler, _114, level(0.0));
    float _119 = _118.x;
    float _133 = fma(_119, View.View_InvDeviceZToWorldZTransform.x, View.View_InvDeviceZToWorldZTransform.y) + (1.0 / fma(_119, View.View_InvDeviceZToWorldZTransform.z, -View.View_InvDeviceZToWorldZTransform.w));
    float4 _147 = View.View_ScreenToWorld * float4((fma(gl_FragCoord.xy, View.View_BufferSizeAndInvSize.zw, -View.View_ScreenPositionScaleBias.wz) / View.View_ScreenPositionScaleBias.xy) * float2(_133), _133, 1.0);
    float3 _148 = _147.xyz;
    float3 _152 = _Globals.LightPositionAndInvRadius.xyz - _148;
    float _158 = length(_152);
    bool _160 = (_158 * _Globals.LightPositionAndInvRadius.w) < 1.0;
    float _207;
    if (_160)
    {
        float3 _165 = abs(_152);
        float _166 = _165.x;
        float _167 = _165.y;
        float _168 = _165.z;
        float _170 = fast::max(_166, fast::max(_167, _168));
        int _189;
        if (_170 == _166)
        {
            _189 = (_166 == _152.x) ? 0 : 1;
        }
        else
        {
            int _185;
            if (_170 == _167)
            {
                _185 = (_167 == _152.y) ? 2 : 3;
            }
            else
            {
                _185 = (_168 == _152.z) ? 4 : 5;
            }
            _189 = _185;
        }
        float4 _196 = _Globals.ShadowViewProjectionMatrices[_189] * float4(_147.xyz, 1.0);
        float _198 = _196.w;
        _207 = ShadowDepthCubeTexture.sample_compare(ShadowDepthCubeTextureSampler, (_152 / float3(_158)), (_196.z / _198) + ((-_Globals.PointLightDepthBiasAndProjParameters.x) / _198), level(0.0));
    }
    else
    {
        _207 = 1.0;
    }
    float _213 = fast::clamp(fma(_207 - 0.5, _Globals.ShadowSharpen, 0.5), 0.0, 1.0);
    float _218 = sqrt(mix(1.0, _213 * _213, _Globals.ShadowFadeFraction));
    float4 _219;
    _219.z = _218;
    float4 _220 = float4(float3(1.0).x, float3(1.0).y, _219.z, float3(1.0).z);
    float3 _236 = fast::normalize(fma(SceneTexturesStruct_GBufferATexture.sample(SceneTexturesStruct_GBufferATextureSampler, _114, level(0.0)).xyz, float3(2.0), float3(-1.0)));
    uint _240 = uint(round(SceneTexturesStruct_GBufferBTexture.sample(SceneTexturesStruct_GBufferBTextureSampler, _114, level(0.0)).w * 255.0));
    bool _248 = (_240 & 15u) == 5u;
    float _448;
    if (_248)
    {
        float4 _260 = SSProfilesTexture.read(uint2(int3(1, int(uint(fma(select(float4(0.0), SceneTexturesStruct_GBufferDTexture.sample(SceneTexturesStruct_GBufferDTextureSampler, _114, level(0.0)), bool4(!(((_240 & 4294967280u) & 16u) != 0u))).x, 255.0, 0.5))), 0).xy), 0);
        float _263 = _260.y * 0.5;
        float3 _266 = fma(-_236, float3(_263), _148);
        float _274 = pow(fast::clamp(dot(-(_152 * float3(rsqrt(dot(_152, _152)))), _236), 0.0, 1.0), 1.0);
        float _445;
        if (_160)
        {
            float3 _278 = _152 / float3(_158);
            float3 _280 = fast::normalize(cross(_278, float3(0.0, 0.0, 1.0)));
            float3 _284 = float3(_Globals.InvShadowmapResolution);
            float3 _285 = _280 * _284;
            float3 _286 = cross(_280, _278) * _284;
            float3 _287 = abs(_278);
            float _288 = _287.x;
            float _289 = _287.y;
            float _290 = _287.z;
            float _292 = fast::max(_288, fast::max(_289, _290));
            int _311;
            if (_292 == _288)
            {
                _311 = (_288 == _278.x) ? 0 : 1;
            }
            else
            {
                int _307;
                if (_292 == _289)
                {
                    _307 = (_289 == _278.y) ? 2 : 3;
                }
                else
                {
                    _307 = (_290 == _278.z) ? 4 : 5;
                }
                _311 = _307;
            }
            float4 _318 = _Globals.ShadowViewProjectionMatrices[_311] * float4(_266, 1.0);
            float _323 = _260.x * (10.0 / _Globals.LightPositionAndInvRadius.w);
            float _457 = -_Globals.PointLightDepthBiasAndProjParameters.w;
            float _328 = 1.0 / fma(_318.z / _318.w, _Globals.PointLightDepthBiasAndProjParameters.z, _457);
            float _341 = fma(_328, _Globals.LightPositionAndInvRadius.w, -((1.0 / fma(float4(ShadowDepthCubeTexture.sample(ShadowDepthTextureSampler, fma(_286, float3(2.5), _278), level(0.0))).x, _Globals.PointLightDepthBiasAndProjParameters.z, _457)) * _Globals.LightPositionAndInvRadius.w));
            float _342 = _341 * _323;
            float _363 = fma(_328, _Globals.LightPositionAndInvRadius.w, -((1.0 / fma(float4(ShadowDepthCubeTexture.sample(ShadowDepthTextureSampler, fma(_286, float3(0.77254199981689453125), fma(_285, float3(2.3776409626007080078125), _278)), level(0.0))).x, _Globals.PointLightDepthBiasAndProjParameters.z, _457)) * _Globals.LightPositionAndInvRadius.w));
            float _364 = _363 * _323;
            float _386 = fma(_328, _Globals.LightPositionAndInvRadius.w, -((1.0 / fma(float4(ShadowDepthCubeTexture.sample(ShadowDepthTextureSampler, fma(_286, float3(-2.0225429534912109375), fma(_285, float3(1.46946299076080322265625), _278)), level(0.0))).x, _Globals.PointLightDepthBiasAndProjParameters.z, _457)) * _Globals.LightPositionAndInvRadius.w));
            float _387 = _386 * _323;
            float _409 = fma(_328, _Globals.LightPositionAndInvRadius.w, -((1.0 / fma(float4(ShadowDepthCubeTexture.sample(ShadowDepthTextureSampler, fma(_286, float3(-2.02254199981689453125), fma(_285, float3(-1.46946299076080322265625), _278)), level(0.0))).x, _Globals.PointLightDepthBiasAndProjParameters.z, _457)) * _Globals.LightPositionAndInvRadius.w));
            float _410 = _409 * _323;
            float _432 = fma(_328, _Globals.LightPositionAndInvRadius.w, -((1.0 / fma(float4(ShadowDepthCubeTexture.sample(ShadowDepthTextureSampler, fma(_286, float3(0.772543013095855712890625), fma(_285, float3(-2.3776409626007080078125), _278)), level(0.0))).x, _Globals.PointLightDepthBiasAndProjParameters.z, _457)) * _Globals.LightPositionAndInvRadius.w));
            float _433 = _432 * _323;
            _445 = (((((fast::clamp(abs((_342 > 0.0) ? fma(_341, _323, _263) : fast::max(0.0, fma(_342, _274, _263))), 0.1500000059604644775390625, 5.0) + 0.25) + (fast::clamp(abs((_364 > 0.0) ? fma(_363, _323, _263) : fast::max(0.0, fma(_364, _274, _263))), 0.1500000059604644775390625, 5.0) + 0.25)) + (fast::clamp(abs((_387 > 0.0) ? fma(_386, _323, _263) : fast::max(0.0, fma(_387, _274, _263))), 0.1500000059604644775390625, 5.0) + 0.25)) + (fast::clamp(abs((_410 > 0.0) ? fma(_409, _323, _263) : fast::max(0.0, fma(_410, _274, _263))), 0.1500000059604644775390625, 5.0) + 0.25)) + (fast::clamp(abs((_433 > 0.0) ? fma(_432, _323, _263) : fast::max(0.0, fma(_433, _274, _263))), 0.1500000059604644775390625, 5.0) + 0.25)) * 0.20000000298023223876953125;
        }
        else
        {
            _445 = 1.0;
        }
        _448 = fma(-_445, 0.20000000298023223876953125, 1.0);
    }
    else
    {
        _448 = 1.0;
    }
    _220.w = _248 ? sqrt(_448) : _218;
    out.out_var_SV_Target0 = _220;
    return out;
}

