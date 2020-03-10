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

constant float4 _90 = {};

struct main0_out
{
    float4 out_var_TEXCOORD10_centroid [[user(locn0)]];
    float4 out_var_TEXCOORD11_centroid [[user(locn1)]];
    float out_var_TEXCOORD6 [[user(locn2)]];
    float3 out_var_TEXCOORD7 [[user(locn3)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 in_var_PN_POSITION_0 [[attribute(10)]];
    float4 in_var_PN_POSITION_1 [[attribute(11)]];
    float4 in_var_PN_POSITION_2 [[attribute(12)]];
    float4 in_var_TEXCOORD10_centroid [[attribute(16)]];
    float4 in_var_TEXCOORD11_centroid [[attribute(17)]];
};

struct main0_patchIn
{
    float4 in_var_PN_POSITION9 [[attribute(13)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant type_ShadowDepthPass& ShadowDepthPass [[buffer(0)]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 3> _93 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_TEXCOORD10_centroid, patchIn.gl_in[1].in_var_TEXCOORD10_centroid, patchIn.gl_in[2].in_var_TEXCOORD10_centroid });
    spvUnsafeArray<float4, 3> _94 = spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_TEXCOORD11_centroid, patchIn.gl_in[1].in_var_TEXCOORD11_centroid, patchIn.gl_in[2].in_var_TEXCOORD11_centroid });
    spvUnsafeArray<spvUnsafeArray<float4, 3>, 3> _101 = spvUnsafeArray<spvUnsafeArray<float4, 3>, 3>({ spvUnsafeArray<float4, 3>({ patchIn.gl_in[0].in_var_PN_POSITION_0, patchIn.gl_in[0].in_var_PN_POSITION_1, patchIn.gl_in[0].in_var_PN_POSITION_2 }), spvUnsafeArray<float4, 3>({ patchIn.gl_in[1].in_var_PN_POSITION_0, patchIn.gl_in[1].in_var_PN_POSITION_1, patchIn.gl_in[1].in_var_PN_POSITION_2 }), spvUnsafeArray<float4, 3>({ patchIn.gl_in[2].in_var_PN_POSITION_0, patchIn.gl_in[2].in_var_PN_POSITION_1, patchIn.gl_in[2].in_var_PN_POSITION_2 }) });
    float _119 = gl_TessCoord.x * gl_TessCoord.x;
    float _120 = gl_TessCoord.y * gl_TessCoord.y;
    float _121 = gl_TessCoord.z * gl_TessCoord.z;
    float4 _127 = float4(gl_TessCoord.x);
    float4 _131 = float4(gl_TessCoord.y);
    float4 _136 = float4(gl_TessCoord.z);
    float4 _139 = float4(_119 * 3.0);
    float4 _143 = float4(_120 * 3.0);
    float4 _150 = float4(_121 * 3.0);
    float4 _164 = ((((((((((_101[0][0] * float4(_119)) * _127) + ((_101[1][0] * float4(_120)) * _131)) + ((_101[2][0] * float4(_121)) * _136)) + ((_101[0][1] * _139) * _131)) + ((_101[0][2] * _143) * _127)) + ((_101[1][1] * _143) * _136)) + ((_101[1][2] * _150) * _131)) + ((_101[2][1] * _150) * _127)) + ((_101[2][2] * _139) * _136)) + ((((patchIn.in_var_PN_POSITION9 * float4(6.0)) * _136) * _127) * _131);
    float3 _179 = ((_93[0].xyz * float3(gl_TessCoord.x)) + (_93[1].xyz * float3(gl_TessCoord.y))).xyz + (_93[2].xyz * float3(gl_TessCoord.z));
    float4 _182 = ((_94[0] * _127) + (_94[1] * _131)) + (_94[2] * _136);
    float4x4 _92 = ShadowDepthPass.ShadowDepthPass_ViewMatrix;
    float4 _189 = ShadowDepthPass.ShadowDepthPass_ProjectionMatrix * float4(_164.x, _164.y, _164.z, _164.w);
    float4 _200;
    if ((ShadowDepthPass.ShadowDepthPass_bClampToNearPlane > 0.0) && (_189.z < 0.0))
    {
        float4 _198 = _189;
        _198.z = 9.9999999747524270787835121154785e-07;
        float4 _199 = _198;
        _199.w = 1.0;
        _200 = _199;
    }
    else
    {
        _200 = _189;
    }
    float _209 = abs(dot(float3(_92[0u].z, _92[1u].z, _92[2u].z), _182.xyz));
    float4 _234 = _200;
    _234.z = ((_200.z * ShadowDepthPass.ShadowDepthPass_ShadowParams.w) + ((ShadowDepthPass.ShadowDepthPass_ShadowParams.y * fast::clamp((abs(_209) > 0.0) ? (sqrt(fast::clamp(1.0 - (_209 * _209), 0.0, 1.0)) / _209) : ShadowDepthPass.ShadowDepthPass_ShadowParams.z, 0.0, ShadowDepthPass.ShadowDepthPass_ShadowParams.z)) + ShadowDepthPass.ShadowDepthPass_ShadowParams.x)) * _200.w;
    out.out_var_TEXCOORD10_centroid = float4(_179.x, _179.y, _179.z, _90.w);
    out.out_var_TEXCOORD11_centroid = _182;
    out.out_var_TEXCOORD6 = 0.0;
    out.out_var_TEXCOORD7 = _164.xyz;
    out.gl_Position = _234;
    return out;
}

