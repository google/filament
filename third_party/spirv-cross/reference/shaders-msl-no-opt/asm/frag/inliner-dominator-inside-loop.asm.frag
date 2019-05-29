#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOutput
{
    float4 HPosition;
    float4 Uv_EdgeDistance1;
    float4 UvStuds_EdgeDistance2;
    float4 Color;
    float4 LightPosition_Fog;
    float4 View_Depth;
    float4 Normal_SpecPower;
    float3 Tangent;
    float4 PosLightSpace_Reflectance;
    float studIndex;
};

struct Surface
{
    float3 albedo;
    float3 normal;
    float specular;
    float gloss;
    float reflectance;
    float opacity;
};

struct SurfaceInput
{
    float4 Color;
    float2 Uv;
    float2 UvStuds;
};

struct Globals
{
    float4x4 ViewProjection;
    float4 ViewRight;
    float4 ViewUp;
    float4 ViewDir;
    float3 CameraPosition;
    float3 AmbientColor;
    float3 Lamp0Color;
    float3 Lamp0Dir;
    float3 Lamp1Color;
    float4 FogParams;
    float3 FogColor;
    float4 LightBorder;
    float4 LightConfig0;
    float4 LightConfig1;
    float4 LightConfig2;
    float4 LightConfig3;
    float4 RefractionBias_FadeDistance_GlowFactor;
    float4 OutlineBrightness_ShadowInfo;
    float4 ShadowMatrix0;
    float4 ShadowMatrix1;
    float4 ShadowMatrix2;
};

struct CB0
{
    Globals CB0;
};

struct Params
{
    float4 LqmatFarTilingFactor;
};

struct CB2
{
    Params CB2;
};

constant VertexOutput _121 = {};
constant SurfaceInput _122 = {};
constant float2 _123 = {};
constant float4 _124 = {};
constant Surface _125 = {};
constant float4 _192 = {};
constant float4 _219 = {};
constant float4 _297 = {};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

struct main0_in
{
    float4 IN_Uv_EdgeDistance1 [[user(locn0)]];
    float4 IN_UvStuds_EdgeDistance2 [[user(locn1)]];
    float4 IN_Color [[user(locn2)]];
    float4 IN_LightPosition_Fog [[user(locn3)]];
    float4 IN_View_Depth [[user(locn4)]];
    float4 IN_Normal_SpecPower [[user(locn5)]];
    float3 IN_Tangent [[user(locn6)]];
    float4 IN_PosLightSpace_Reflectance [[user(locn7)]];
    float IN_studIndex [[user(locn8)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant CB0& _19 [[buffer(0)]], texture2d<float> StudsMapTexture [[texture(0)]], texture2d<float> ShadowMapTexture [[texture(1)]], texturecube<float> EnvironmentMapTexture [[texture(2)]], texture2d<float> DiffuseMapTexture [[texture(3)]], texture2d<float> NormalMapTexture [[texture(4)]], texture2d<float> SpecularMapTexture [[texture(5)]], texture3d<float> LightMapTexture [[texture(6)]], texture2d<float> NormalDetailMapTexture [[texture(8)]], sampler StudsMapSampler [[sampler(0)]], sampler ShadowMapSampler [[sampler(1)]], sampler EnvironmentMapSampler [[sampler(2)]], sampler DiffuseMapSampler [[sampler(3)]], sampler NormalMapSampler [[sampler(4)]], sampler SpecularMapSampler [[sampler(5)]], sampler LightMapSampler [[sampler(6)]], sampler NormalDetailMapSampler [[sampler(8)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    VertexOutput _128 = _121;
    _128.HPosition = gl_FragCoord;
    VertexOutput _130 = _128;
    _130.Uv_EdgeDistance1 = in.IN_Uv_EdgeDistance1;
    VertexOutput _132 = _130;
    _132.UvStuds_EdgeDistance2 = in.IN_UvStuds_EdgeDistance2;
    VertexOutput _134 = _132;
    _134.Color = in.IN_Color;
    VertexOutput _136 = _134;
    _136.LightPosition_Fog = in.IN_LightPosition_Fog;
    VertexOutput _138 = _136;
    _138.View_Depth = in.IN_View_Depth;
    VertexOutput _140 = _138;
    _140.Normal_SpecPower = in.IN_Normal_SpecPower;
    VertexOutput _142 = _140;
    _142.Tangent = in.IN_Tangent;
    VertexOutput _144 = _142;
    _144.PosLightSpace_Reflectance = in.IN_PosLightSpace_Reflectance;
    VertexOutput _146 = _144;
    _146.studIndex = in.IN_studIndex;
    SurfaceInput _147 = _122;
    _147.Color = in.IN_Color;
    SurfaceInput _149 = _147;
    _149.Uv = in.IN_Uv_EdgeDistance1.xy;
    SurfaceInput _151 = _149;
    _151.UvStuds = in.IN_UvStuds_EdgeDistance2.xy;
    SurfaceInput _156 = _151;
    _156.UvStuds.y = (fract(_151.UvStuds.y) + in.IN_studIndex) * 0.25;
    float _163 = _146.View_Depth.w * _19.CB0.RefractionBias_FadeDistance_GlowFactor.y;
    float _165 = fast::clamp(1.0 - _163, 0.0, 1.0);
    float2 _166 = in.IN_Uv_EdgeDistance1.xy * 1.0;
    bool _173;
    float4 _193;
    do
    {
        _173 = 0.0 == 0.0;
        if (_173)
        {
            _193 = DiffuseMapTexture.sample(DiffuseMapSampler, _166);
            break;
        }
        else
        {
            float _180 = 1.0 / (1.0 - 0.0);
            _193 = mix(DiffuseMapTexture.sample(DiffuseMapSampler, (_166 * 0.25)), DiffuseMapTexture.sample(DiffuseMapSampler, _166), float4(fast::clamp((fast::clamp(1.0 - (_146.View_Depth.w * 0.00333332992158830165863037109375), 0.0, 1.0) * _180) - (0.0 * _180), 0.0, 1.0)));
            break;
        }
        _193 = _192;
        break;
    } while (false);
    float4 _194 = _193 * 1.0;
    float4 _220;
    do
    {
        if (_173)
        {
            _220 = NormalMapTexture.sample(NormalMapSampler, _166);
            break;
        }
        else
        {
            float _207 = 1.0 / (1.0 - 0.0);
            _220 = mix(NormalMapTexture.sample(NormalMapSampler, (_166 * 0.25)), NormalMapTexture.sample(NormalMapSampler, _166), float4(fast::clamp((_165 * _207) - (0.0 * _207), 0.0, 1.0)));
            break;
        }
        _220 = _219;
        break;
    } while (false);
    float2 _223 = float2(1.0);
    float2 _224 = (_220.wy * 2.0) - _223;
    float3 _232 = float3(_224, sqrt(fast::clamp(1.0 + dot(-_224, _224), 0.0, 1.0)));
    float2 _240 = (NormalDetailMapTexture.sample(NormalDetailMapSampler, (_166 * 0.0)).wy * 2.0) - _223;
    float2 _252 = _232.xy + (float3(_240, sqrt(fast::clamp(1.0 + dot(-_240, _240), 0.0, 1.0))).xy * 0.0);
    float3 _253 = float3(_252.x, _252.y, _232.z);
    float2 _255 = _253.xy * _165;
    float3 _256 = float3(_255.x, _255.y, _253.z);
    float3 _271 = ((in.IN_Color.xyz * _194.xyz) * (1.0 + (_256.x * 0.300000011920928955078125))) * (StudsMapTexture.sample(StudsMapSampler, _156.UvStuds).x * 2.0);
    float4 _298;
    do
    {
        if (0.75 == 0.0)
        {
            _298 = SpecularMapTexture.sample(SpecularMapSampler, _166);
            break;
        }
        else
        {
            float _285 = 1.0 / (1.0 - 0.75);
            _298 = mix(SpecularMapTexture.sample(SpecularMapSampler, (_166 * 0.25)), SpecularMapTexture.sample(SpecularMapSampler, _166), float4(fast::clamp((_165 * _285) - (0.75 * _285), 0.0, 1.0)));
            break;
        }
        _298 = _297;
        break;
    } while (false);
    float2 _303 = mix(float2(0.800000011920928955078125, 120.0), (_298.xy * float2(2.0, 256.0)) + float2(0.0, 0.00999999977648258209228515625), float2(_165));
    Surface _304 = _125;
    _304.albedo = _271;
    Surface _305 = _304;
    _305.normal = _256;
    float _306 = _303.x;
    Surface _307 = _305;
    _307.specular = _306;
    float _308 = _303.y;
    Surface _309 = _307;
    _309.gloss = _308;
    float _312 = (_298.xy.y * _165) * 0.0;
    Surface _313 = _309;
    _313.reflectance = _312;
    float4 _318 = float4(_271, _146.Color.w);
    float3 _329 = normalize(((in.IN_Tangent * _313.normal.x) + (cross(in.IN_Normal_SpecPower.xyz, in.IN_Tangent) * _313.normal.y)) + (in.IN_Normal_SpecPower.xyz * _313.normal.z));
    float3 _332 = -_19.CB0.Lamp0Dir;
    float _333 = dot(_329, _332);
    float _357 = fast::clamp(dot(step(_19.CB0.LightConfig3.xyz, abs(in.IN_LightPosition_Fog.xyz - _19.CB0.LightConfig2.xyz)), float3(1.0)), 0.0, 1.0);
    float4 _368 = mix(LightMapTexture.sample(LightMapSampler, (in.IN_LightPosition_Fog.xyz.yzx - (in.IN_LightPosition_Fog.xyz.yzx * _357))), _19.CB0.LightBorder, float4(_357));
    float2 _376 = ShadowMapTexture.sample(ShadowMapSampler, in.IN_PosLightSpace_Reflectance.xyz.xy).xy;
    float _392 = (1.0 - (((step(_376.x, in.IN_PosLightSpace_Reflectance.xyz.z) * fast::clamp(9.0 - (20.0 * abs(in.IN_PosLightSpace_Reflectance.xyz.z - 0.5)), 0.0, 1.0)) * _376.y) * _19.CB0.OutlineBrightness_ShadowInfo.w)) * _368.w;
    float3 _403 = mix(_318.xyz, EnvironmentMapTexture.sample(EnvironmentMapSampler, reflect(-in.IN_View_Depth.xyz, _329)).xyz, float3(_312));
    float4 _404 = float4(_403.x, _403.y, _403.z, _318.w);
    float3 _422 = (((_19.CB0.AmbientColor + (((_19.CB0.Lamp0Color * fast::clamp(_333, 0.0, 1.0)) + (_19.CB0.Lamp1Color * fast::max(-_333, 0.0))) * _392)) + _368.xyz) * _404.xyz) + (_19.CB0.Lamp0Color * (((step(0.0, _333) * _306) * _392) * pow(fast::clamp(dot(_329, normalize(_332 + normalize(in.IN_View_Depth.xyz))), 0.0, 1.0), _308)));
    float4 _425 = float4(_422.x, _422.y, _422.z, _124.w);
    _425.w = _404.w;
    float2 _435 = fast::min(in.IN_Uv_EdgeDistance1.wz, in.IN_UvStuds_EdgeDistance2.wz);
    float _439 = fast::min(_435.x, _435.y) / _163;
    float3 _445 = _425.xyz * fast::clamp((fast::clamp((_163 * _19.CB0.OutlineBrightness_ShadowInfo.x) + _19.CB0.OutlineBrightness_ShadowInfo.y, 0.0, 1.0) * (1.5 - _439)) + _439, 0.0, 1.0);
    float4 _446 = float4(_445.x, _445.y, _445.z, _425.w);
    float3 _453 = mix(_19.CB0.FogColor, _446.xyz, float3(fast::clamp(_146.LightPosition_Fog.w, 0.0, 1.0)));
    out._entryPointOutput = float4(_453.x, _453.y, _453.z, _446.w);
    return out;
}

