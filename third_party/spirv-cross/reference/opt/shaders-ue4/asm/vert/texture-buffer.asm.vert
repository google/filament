#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

struct type_MobileShadowDepthPass
{
    float PrePadding_MobileShadowDepthPass_0;
    float PrePadding_MobileShadowDepthPass_4;
    float PrePadding_MobileShadowDepthPass_8;
    float PrePadding_MobileShadowDepthPass_12;
    float PrePadding_MobileShadowDepthPass_16;
    float PrePadding_MobileShadowDepthPass_20;
    float PrePadding_MobileShadowDepthPass_24;
    float PrePadding_MobileShadowDepthPass_28;
    float PrePadding_MobileShadowDepthPass_32;
    float PrePadding_MobileShadowDepthPass_36;
    float PrePadding_MobileShadowDepthPass_40;
    float PrePadding_MobileShadowDepthPass_44;
    float PrePadding_MobileShadowDepthPass_48;
    float PrePadding_MobileShadowDepthPass_52;
    float PrePadding_MobileShadowDepthPass_56;
    float PrePadding_MobileShadowDepthPass_60;
    float PrePadding_MobileShadowDepthPass_64;
    float PrePadding_MobileShadowDepthPass_68;
    float PrePadding_MobileShadowDepthPass_72;
    float PrePadding_MobileShadowDepthPass_76;
    float4x4 MobileShadowDepthPass_ProjectionMatrix;
    float2 MobileShadowDepthPass_ShadowParams;
    float MobileShadowDepthPass_bClampToNearPlane;
    float PrePadding_MobileShadowDepthPass_156;
    float4x4 MobileShadowDepthPass_ShadowViewProjectionMatrices[6];
};

struct type_EmitterDynamicUniforms
{
    float2 EmitterDynamicUniforms_LocalToWorldScale;
    float EmitterDynamicUniforms_EmitterInstRandom;
    float PrePadding_EmitterDynamicUniforms_12;
    float4 EmitterDynamicUniforms_AxisLockRight;
    float4 EmitterDynamicUniforms_AxisLockUp;
    float4 EmitterDynamicUniforms_DynamicColor;
    float4 EmitterDynamicUniforms_MacroUVParameters;
};

struct type_EmitterUniforms
{
    float4 EmitterUniforms_ColorCurve;
    float4 EmitterUniforms_ColorScale;
    float4 EmitterUniforms_ColorBias;
    float4 EmitterUniforms_MiscCurve;
    float4 EmitterUniforms_MiscScale;
    float4 EmitterUniforms_MiscBias;
    float4 EmitterUniforms_SizeBySpeed;
    float4 EmitterUniforms_SubImageSize;
    float4 EmitterUniforms_TangentSelector;
    packed_float3 EmitterUniforms_CameraFacingBlend;
    float EmitterUniforms_RemoveHMDRoll;
    float EmitterUniforms_RotationRateScale;
    float EmitterUniforms_RotationBias;
    float EmitterUniforms_CameraMotionBlurAmount;
    float PrePadding_EmitterUniforms_172;
    float2 EmitterUniforms_PivotOffset;
};

struct type_Globals
{
    uint ParticleIndicesOffset;
};

struct main0_out
{
    float out_var_TEXCOORD6 [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 in_var_ATTRIBUTE0 [[attribute(0)]];
};

// Returns 2D texture coords corresponding to 1D texel buffer coords
static inline __attribute__((always_inline))
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

vertex main0_out main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], constant type_Primitive& Primitive [[buffer(1)]], constant type_MobileShadowDepthPass& MobileShadowDepthPass [[buffer(2)]], constant type_EmitterDynamicUniforms& EmitterDynamicUniforms [[buffer(3)]], constant type_EmitterUniforms& EmitterUniforms [[buffer(4)]], constant type_Globals& _Globals [[buffer(5)]], texture2d<float> ParticleIndices [[texture(0)]], texture2d<float> PositionTexture [[texture(1)]], texture2d<float> VelocityTexture [[texture(2)]], texture2d<float> AttributesTexture [[texture(3)]], texture2d<float> CurveTexture [[texture(4)]], sampler PositionTextureSampler [[sampler(0)]], sampler VelocityTextureSampler [[sampler(1)]], sampler AttributesTextureSampler [[sampler(2)]], sampler CurveTextureSampler [[sampler(3)]], uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    float2 _133 = ParticleIndices.read(spvTexelBufferCoord((_Globals.ParticleIndicesOffset + ((gl_InstanceIndex * 16u) + (gl_VertexIndex / 4u))))).xy;
    float4 _137 = PositionTexture.sample(PositionTextureSampler, _133, level(0.0));
    float4 _145 = AttributesTexture.sample(AttributesTextureSampler, _133, level(0.0));
    float _146 = _137.w;
    float3 _158 = float3x3(Primitive.Primitive_LocalToWorld[0].xyz, Primitive.Primitive_LocalToWorld[1].xyz, Primitive.Primitive_LocalToWorld[2].xyz) * VelocityTexture.sample(VelocityTextureSampler, _133, level(0.0)).xyz;
    float3 _160 = normalize(_158 + float3(0.0, 0.0, 9.9999997473787516355514526367188e-05));
    float2 _204 = ((((_145.xy + float2((_145.x < 0.5) ? 0.0 : (-0.5), (_145.y < 0.5) ? 0.0 : (-0.5))) * float2(2.0)) * (((CurveTexture.sample(CurveTextureSampler, (EmitterUniforms.EmitterUniforms_MiscCurve.xy + (EmitterUniforms.EmitterUniforms_MiscCurve.zw * float2(_146))), level(0.0)) * EmitterUniforms.EmitterUniforms_MiscScale) + EmitterUniforms.EmitterUniforms_MiscBias).xy * EmitterDynamicUniforms.EmitterDynamicUniforms_LocalToWorldScale)) * fast::min(fast::max(EmitterUniforms.EmitterUniforms_SizeBySpeed.xy * float2(length(_158)), float2(1.0)), EmitterUniforms.EmitterUniforms_SizeBySpeed.zw)) * float2(step(_146, 1.0));
    float3 _239 = float4((((Primitive.Primitive_LocalToWorld[0u].xyz * _137.xxx) + (Primitive.Primitive_LocalToWorld[1u].xyz * _137.yyy)) + (Primitive.Primitive_LocalToWorld[2u].xyz * _137.zzz)) + (Primitive.Primitive_LocalToWorld[3u].xyz + float3(View.View_PreViewTranslation)), 1.0).xyz;
    float3 _242 = float3(EmitterUniforms.EmitterUniforms_RemoveHMDRoll);
    float3 _251 = mix(mix(float3(View.View_ViewRight), float3(View.View_HMDViewNoRollRight), _242), EmitterDynamicUniforms.EmitterDynamicUniforms_AxisLockRight.xyz, float3(EmitterDynamicUniforms.EmitterDynamicUniforms_AxisLockRight.w));
    float3 _259 = mix(-mix(float3(View.View_ViewUp), float3(View.View_HMDViewNoRollUp), _242), EmitterDynamicUniforms.EmitterDynamicUniforms_AxisLockUp.xyz, float3(EmitterDynamicUniforms.EmitterDynamicUniforms_AxisLockUp.w));
    float3 _260 = float3(View.View_TranslatedWorldCameraOrigin) - _239;
    float _261 = dot(_260, _260);
    float3 _265 = _260 / float3(sqrt(fast::max(_261, 0.00999999977648258209228515625)));
    float3 _335;
    float3 _336;
    if (EmitterUniforms.EmitterUniforms_CameraFacingBlend[0] > 0.0)
    {
        float3 _279 = cross(_265, float3(0.0, 0.0, 1.0));
        float3 _284 = _279 / float3(sqrt(fast::max(dot(_279, _279), 0.00999999977648258209228515625)));
        float3 _286 = float3(fast::clamp((_261 * EmitterUniforms.EmitterUniforms_CameraFacingBlend[1]) - EmitterUniforms.EmitterUniforms_CameraFacingBlend[2], 0.0, 1.0));
        _335 = normalize(mix(_251, _284, _286));
        _336 = normalize(mix(_259, cross(_265, _284), _286));
    }
    else
    {
        float3 _333;
        float3 _334;
        if (EmitterUniforms.EmitterUniforms_TangentSelector.y > 0.0)
        {
            float3 _297 = cross(_265, _160);
            _333 = _297 / float3(sqrt(fast::max(dot(_297, _297), 0.00999999977648258209228515625)));
            _334 = -_160;
        }
        else
        {
            float3 _331;
            float3 _332;
            if (EmitterUniforms.EmitterUniforms_TangentSelector.z > 0.0)
            {
                float3 _310 = cross(EmitterDynamicUniforms.EmitterDynamicUniforms_AxisLockRight.xyz, _265);
                _331 = EmitterDynamicUniforms.EmitterDynamicUniforms_AxisLockRight.xyz;
                _332 = -(_310 / float3(sqrt(fast::max(dot(_310, _310), 0.00999999977648258209228515625))));
            }
            else
            {
                float3 _329;
                float3 _330;
                if (EmitterUniforms.EmitterUniforms_TangentSelector.w > 0.0)
                {
                    float3 _322 = cross(_265, float3(0.0, 0.0, 1.0));
                    float3 _327 = _322 / float3(sqrt(fast::max(dot(_322, _322), 0.00999999977648258209228515625)));
                    _329 = _327;
                    _330 = cross(_265, _327);
                }
                else
                {
                    _329 = _251;
                    _330 = _259;
                }
                _331 = _329;
                _332 = _330;
            }
            _333 = _331;
            _334 = _332;
        }
        _335 = _333;
        _336 = _334;
    }
    float _339 = ((_145.z + ((_145.w * EmitterUniforms.EmitterUniforms_RotationRateScale) * _146)) * 6.283185482025146484375) + EmitterUniforms.EmitterUniforms_RotationBias;
    float3 _342 = float3(sin(_339));
    float3 _344 = float3(cos(_339));
    float4 _371 = float4(_239 + ((float3(_204.x * (in.in_var_ATTRIBUTE0.x + EmitterUniforms.EmitterUniforms_PivotOffset.x)) * ((_342 * _336) + (_344 * _335))) + (float3(_204.y * (in.in_var_ATTRIBUTE0.y + EmitterUniforms.EmitterUniforms_PivotOffset.y)) * ((_344 * _336) - (_342 * _335)))), 1.0);
    float4 _375 = MobileShadowDepthPass.MobileShadowDepthPass_ProjectionMatrix * float4(_371.x, _371.y, _371.z, _371.w);
    float4 _386;
    if ((MobileShadowDepthPass.MobileShadowDepthPass_bClampToNearPlane > 0.0) && (_375.z < 0.0))
    {
        float4 _384 = _375;
        _384.z = 9.9999999747524270787835121154785e-07;
        float4 _385 = _384;
        _385.w = 1.0;
        _386 = _385;
    }
    else
    {
        _386 = _375;
    }
    float4 _396 = _386;
    _396.z = ((_386.z * MobileShadowDepthPass.MobileShadowDepthPass_ShadowParams.y) + MobileShadowDepthPass.MobileShadowDepthPass_ShadowParams.x) * _386.w;
    out.out_var_TEXCOORD6 = 0.0;
    out.gl_Position = _396;
    return out;
}

