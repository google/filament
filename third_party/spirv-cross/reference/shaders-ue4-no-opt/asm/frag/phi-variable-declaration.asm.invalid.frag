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

struct type_PrimitiveDither
{
    float PrimitiveDither_LODFactor;
};

struct type_PrimitiveFade
{
    float2 PrimitiveFade_FadeTimeScaleBias;
};

struct type_Material
{
    float4 Material_VectorExpressions[9];
    float4 Material_ScalarExpressions[3];
};

constant float _98 = {};
constant float _103 = {};

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
    float gl_FragDepth [[depth(less)]];
};

struct main0_in
{
    float4 in_var_TEXCOORD6 [[user(locn0)]];
    float4 in_var_TEXCOORD7 [[user(locn1)]];
    float4 in_var_TEXCOORD10_centroid [[user(locn2)]];
    float4 in_var_TEXCOORD11_centroid [[user(locn3)]];
    float4 in_var_TEXCOORD0_0 [[user(locn4)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant type_View& View [[buffer(0)]], constant type_PrimitiveDither& PrimitiveDither [[buffer(1)]], constant type_PrimitiveFade& PrimitiveFade [[buffer(2)]], constant type_Material& Material [[buffer(3)]], texture2d<float> Material_Texture2D_0 [[texture(0)]], texture2d<float> Material_Texture2D_3 [[texture(1)]], sampler Material_Texture2D_0Sampler [[sampler(0)]], sampler Material_Texture2D_3Sampler [[sampler(1)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 1> in_var_TEXCOORD0 = {};
    in_var_TEXCOORD0[0] = in.in_var_TEXCOORD0_0;
    float2 _135 = gl_FragCoord.xy - View.View_ViewRectMin.xy;
    float4 _140 = float4(_103, _103, gl_FragCoord.z, 1.0) * float4(gl_FragCoord.w);
    float4 _144 = View.View_SVPositionToTranslatedWorld * float4(gl_FragCoord.xyz, 1.0);
    float3 _148 = _144.xyz / float3(_144.w);
    float3 _149 = _148 - float3(View.View_PreViewTranslation);
    float3 _151 = normalize(-_148);
    float3 _152 = _151 * float3x3(in.in_var_TEXCOORD10_centroid.xyz, cross(in.in_var_TEXCOORD11_centroid.xyz, in.in_var_TEXCOORD10_centroid.xyz) * float3(in.in_var_TEXCOORD11_centroid.w), in.in_var_TEXCOORD11_centroid.xyz);
    float _170 = mix(Material.Material_ScalarExpressions[0].y, Material.Material_ScalarExpressions[0].z, fast::min(fast::max(abs(dot(_151, in.in_var_TEXCOORD11_centroid.xyz)), 0.0), 1.0));
    float _171 = floor(_170);
    float _172 = 1.0 / _170;
    float2 _174 = (float2(Material.Material_ScalarExpressions[0].x) * ((_152.xy * float2(-1.0)) / float2(_152.z))) * float2(_172);
    float2 _175 = dfdx(float2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    float2 _176 = dfdy(float2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    float _180_copy;
    float2 _183;
    _183 = float2(0.0);
    float _188;
    float _211;
    float2 _212;
    float _180 = 1.0;
    int _185 = 0;
    float _187 = 1.0;
    float _189 = 1.0;
    for (;;)
    {
        if (float(_185) < (_171 + 2.0))
        {
            _188 = Material_Texture2D_0.sample(Material_Texture2D_0Sampler, (float2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y) + _183), gradient2d(_175, _176)).y;
            if (_180 < _188)
            {
                float _201 = _188 - _180;
                float _203 = _201 / ((_189 - _187) + _201);
                _211 = (_189 * _203) + (_180 * (1.0 - _203));
                _212 = _183 - (float2(_203) * _174);
                break;
            }
            _180_copy = _180;
            _180 -= _172;
            _183 += _174;
            _185++;
            _187 = _188;
            _189 = _180_copy;
            continue;
        }
        else
        {
            _211 = _98;
            _212 = _183;
            break;
        }
    }
    float4 _218 = Material_Texture2D_0.sample(Material_Texture2D_0Sampler, (float2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y) + _212.xy), bias(View.View_MaterialTextureMipBias));
    float2 _229 = _135 + float2(View.View_TemporalAAParams.x);
    float _237 = float((uint(_229.x) + (2u * uint(_229.y))) % 5u);
    float2 _238 = _135 * float2(0.015625);
    float4 _242 = Material_Texture2D_3.sample(Material_Texture2D_3Sampler, _238, bias(View.View_MaterialTextureMipBias));
    float4 _254 = Material_Texture2D_3.sample(Material_Texture2D_3Sampler, _238, bias(View.View_MaterialTextureMipBias));
    float3 _272 = float3(_212, (1.0 - _211) * Material.Material_ScalarExpressions[0].x);
    float2 _275 = dfdx(float2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    float2 _276 = abs(_275);
    float3 _279 = dfdx(_149);
    float2 _283 = dfdy(float2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    float2 _284 = abs(_283);
    float3 _287 = dfdy(_149);
    if (PrimitiveDither.PrimitiveDither_LODFactor != 0.0)
    {
        if (abs(PrimitiveDither.PrimitiveDither_LODFactor) > 0.001000000047497451305389404296875)
        {
            float _317 = fract(cos(dot(floor(gl_FragCoord.xy), float2(347.834503173828125, 3343.28369140625))) * 1000.0);
            if ((float((PrimitiveDither.PrimitiveDither_LODFactor < 0.0) ? ((PrimitiveDither.PrimitiveDither_LODFactor + 1.0) > _317) : (PrimitiveDither.PrimitiveDither_LODFactor < _317)) - 0.001000000047497451305389404296875) < 0.0)
            {
                discard_fragment();
            }
        }
    }
    if ((((_218.z + ((fast::min(fast::max(1.0 - (_218.x * Material.Material_ScalarExpressions[2].y), 0.0), 1.0) + ((_237 + (_242.x * Material.Material_ScalarExpressions[2].z)) * 0.16666667163372039794921875)) + (-0.5))) * ((fast::clamp((View.View_RealTime * PrimitiveFade.PrimitiveFade_FadeTimeScaleBias.x) + PrimitiveFade.PrimitiveFade_FadeTimeScaleBias.y, 0.0, 1.0) + ((_237 + _254.x) * 0.16666667163372039794921875)) + (-0.5))) - 0.33329999446868896484375) < 0.0)
    {
        discard_fragment();
    }
    float2 _351 = ((((in.in_var_TEXCOORD6.xy / float2(in.in_var_TEXCOORD6.w)) - View.View_TemporalAAJitter.xy) - ((in.in_var_TEXCOORD7.xy / float2(in.in_var_TEXCOORD7.w)) - View.View_TemporalAAJitter.zw)) * float2(0.2495000064373016357421875)) + float2(0.49999237060546875);
    out.gl_FragDepth = fast::min(_140.z / (_140.w + (sqrt(dot(_272, _272)) / (fast::max(sqrt(dot(_276, _276)) / sqrt(dot(_279, _279)), sqrt(dot(_284, _284)) / sqrt(dot(_287, _287))) / abs(dot(float3x3(View.View_ViewToTranslatedWorld[0].xyz, View.View_ViewToTranslatedWorld[1].xyz, View.View_ViewToTranslatedWorld[2].xyz) * float3(0.0, 0.0, 1.0), _151))))), gl_FragCoord.z);
    out.out_var_SV_Target0 = float4(_351.x, _351.y, float2(0.0).x, float2(0.0).y);
    return out;
}

