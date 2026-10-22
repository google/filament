#version 450

struct VertexOutput
{
    vec4 HPosition;
    vec4 Uv_EdgeDistance1;
    vec4 UvStuds_EdgeDistance2;
    vec4 Color;
    vec4 LightPosition_Fog;
    vec4 View_Depth;
    vec4 Normal_SpecPower;
    vec3 Tangent;
    vec4 PosLightSpace_Reflectance;
    float studIndex;
};

struct Surface
{
    vec3 albedo;
    vec3 normal;
    float specular;
    float gloss;
    float reflectance;
    float opacity;
};

struct SurfaceInput
{
    vec4 Color;
    vec2 Uv;
    vec2 UvStuds;
};

struct Globals
{
    mat4 ViewProjection;
    vec4 ViewRight;
    vec4 ViewUp;
    vec4 ViewDir;
    vec3 CameraPosition;
    vec3 AmbientColor;
    vec3 Lamp0Color;
    vec3 Lamp0Dir;
    vec3 Lamp1Color;
    vec4 FogParams;
    vec3 FogColor;
    vec4 LightBorder;
    vec4 LightConfig0;
    vec4 LightConfig1;
    vec4 LightConfig2;
    vec4 LightConfig3;
    vec4 RefractionBias_FadeDistance_GlowFactor;
    vec4 OutlineBrightness_ShadowInfo;
    vec4 ShadowMatrix0;
    vec4 ShadowMatrix1;
    vec4 ShadowMatrix2;
};

struct Params
{
    vec4 LqmatFarTilingFactor;
};

VertexOutput _1509;
SurfaceInput _1510;
vec2 _1511;
vec4 _1512;
Surface _1531;
vec4 _1157;
vec4 _1203;
vec4 _1284;

layout(binding = 0, std140) uniform CB0
{
    Globals CB0;
} _24;

uniform sampler2D SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler;
uniform sampler2D SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler;
uniform sampler2D SPIRV_Cross_CombinedNormalDetailMapTextureNormalDetailMapSampler;
uniform sampler2D SPIRV_Cross_CombinedStudsMapTextureStudsMapSampler;
uniform sampler2D SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler;
uniform sampler3D SPIRV_Cross_CombinedLightMapTextureLightMapSampler;
uniform sampler2D SPIRV_Cross_CombinedShadowMapTextureShadowMapSampler;
uniform samplerCube SPIRV_Cross_CombinedEnvironmentMapTextureEnvironmentMapSampler;

layout(location = 0) in vec4 IN_Uv_EdgeDistance1;
layout(location = 1) in vec4 IN_UvStuds_EdgeDistance2;
layout(location = 2) in vec4 IN_Color;
layout(location = 3) in vec4 IN_LightPosition_Fog;
layout(location = 4) in vec4 IN_View_Depth;
layout(location = 5) in vec4 IN_Normal_SpecPower;
layout(location = 6) in vec3 IN_Tangent;
layout(location = 7) in vec4 IN_PosLightSpace_Reflectance;
layout(location = 8) in float IN_studIndex;
layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    VertexOutput _1378;
    _1378.HPosition = gl_FragCoord;
    _1378.Uv_EdgeDistance1 = IN_Uv_EdgeDistance1;
    _1378.UvStuds_EdgeDistance2 = IN_UvStuds_EdgeDistance2;
    _1378.Color = IN_Color;
    _1378.LightPosition_Fog = IN_LightPosition_Fog;
    _1378.View_Depth = IN_View_Depth;
    _1378.Normal_SpecPower = IN_Normal_SpecPower;
    _1378.Tangent = IN_Tangent;
    _1378.PosLightSpace_Reflectance = IN_PosLightSpace_Reflectance;
    _1378.studIndex = IN_studIndex;
    SurfaceInput _1400;
    _1400.Color = IN_Color;
    _1400.Uv = IN_Uv_EdgeDistance1.xy;
    _1400.UvStuds = IN_UvStuds_EdgeDistance2.xy;
    _1400.UvStuds.y = (fract(_1400.UvStuds.y) + IN_studIndex) * 0.25;
    float _987 = clamp(1.0 - (_1378.View_Depth.w * 0.00333332992158830165863037109375), 0.0, 1.0);
    float _978 = _1378.View_Depth.w * _24.CB0.RefractionBias_FadeDistance_GlowFactor.y;
    float _990 = clamp(1.0 - _978, 0.0, 1.0);
    vec2 _1024 = IN_Uv_EdgeDistance1.xy * 1.0;
    bool _1124;
    vec4 _1517;
    for (;;)
    {
        _1124 = 0.0 == 0.0;
        if (_1124)
        {
            _1517 = texture(SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler, _1024);
            break;
        }
        else
        {
            float _1135 = 1.0 / (1.0 - 0.0);
            _1517 = mix(texture(SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler, _1024 * 0.25), texture(SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler, _1024), vec4(clamp((_987 * _1135) - (0.0 * _1135), 0.0, 1.0)));
            break;
        }
        _1517 = _1157;
        break;
    }
    vec4 _1523;
    for (;;)
    {
        if (_1124)
        {
            _1523 = texture(SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler, _1024);
            break;
        }
        else
        {
            float _1181 = 1.0 / (1.0 - 0.0);
            _1523 = mix(texture(SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler, _1024 * 0.25), texture(SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler, _1024), vec4(clamp((_990 * _1181) - (0.0 * _1181), 0.0, 1.0)));
            break;
        }
        _1523 = _1203;
        break;
    }
    vec2 _1212 = vec2(1.0);
    vec2 _1213 = (_1523.wy * 2.0) - _1212;
    vec3 _1224 = vec3(_1213, sqrt(clamp(1.0 + dot(-_1213, _1213), 0.0, 1.0)));
    vec4 _1047 = texture(SPIRV_Cross_CombinedNormalDetailMapTextureNormalDetailMapSampler, _1024 * 0.0);
    vec2 _1231 = (_1047.wy * 2.0) - _1212;
    vec2 _1054 = _1224.xy + (vec3(_1231, sqrt(clamp(1.0 + dot(-_1231, _1231), 0.0, 1.0))).xy * 0.0);
    vec3 _1056 = vec3(_1054.x, _1054.y, _1224.z);
    vec2 _1060 = _1056.xy * _990;
    vec3 _1062 = vec3(_1060.x, _1060.y, _1056.z);
    vec4 _1080 = texture(SPIRV_Cross_CombinedStudsMapTextureStudsMapSampler, _1400.UvStuds);
    vec3 _1085 = ((IN_Color.xyz * (_1517 * 1.0).xyz) * (1.0 + (_1062.x * 0.300000011920928955078125))) * (_1080.x * 2.0);
    vec4 _1530;
    for (;;)
    {
        if (0.75 == 0.0)
        {
            _1530 = texture(SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler, _1024);
            break;
        }
        else
        {
            float _1262 = 1.0 / (1.0 - 0.75);
            _1530 = mix(texture(SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler, _1024 * 0.25), texture(SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler, _1024), vec4(clamp((_990 * _1262) - (0.75 * _1262), 0.0, 1.0)));
            break;
        }
        _1530 = _1284;
        break;
    }
    vec2 _1098 = mix(vec2(0.800000011920928955078125, 120.0), (_1530.xy * vec2(2.0, 256.0)) + vec2(0.0, 0.00999999977648258209228515625), vec2(_990));
    Surface _1438;
    _1438.albedo = _1085;
    _1438.normal = _1062;
    float _1442 = _1098.x;
    _1438.specular = _1442;
    float _1446 = _1098.y;
    _1438.gloss = _1446;
    float _1113 = (_1530.xy.y * _990) * 0.0;
    _1438.reflectance = _1113;
    vec4 _767 = vec4(_1085, _1378.Color.w);
    vec3 _791 = normalize(((IN_Tangent * _1438.normal.x) + (cross(IN_Normal_SpecPower.xyz, IN_Tangent) * _1438.normal.y)) + (IN_Normal_SpecPower.xyz * _1438.normal.z));
    vec3 _795 = -_24.CB0.Lamp0Dir;
    float _796 = dot(_791, _795);
    float _1328 = clamp(dot(step(_24.CB0.LightConfig3.xyz, abs(IN_LightPosition_Fog.xyz - _24.CB0.LightConfig2.xyz)), vec3(1.0)), 0.0, 1.0);
    vec4 _1325 = mix(texture(SPIRV_Cross_CombinedLightMapTextureLightMapSampler, IN_LightPosition_Fog.xyz.yzx - (IN_LightPosition_Fog.xyz.yzx * _1328)), _24.CB0.LightBorder, vec4(_1328));
    vec2 _1341 = texture(SPIRV_Cross_CombinedShadowMapTextureShadowMapSampler, IN_PosLightSpace_Reflectance.xyz.xy).xy;
    float _1356 = (1.0 - (((step(_1341.x, IN_PosLightSpace_Reflectance.xyz.z) * clamp(9.0 - (20.0 * abs(IN_PosLightSpace_Reflectance.xyz.z - 0.5)), 0.0, 1.0)) * _1341.y) * _24.CB0.OutlineBrightness_ShadowInfo.w)) * _1325.w;
    vec3 _846 = mix(_767.xyz, texture(SPIRV_Cross_CombinedEnvironmentMapTextureEnvironmentMapSampler, reflect(-IN_View_Depth.xyz, _791)).xyz, vec3(_1113));
    vec3 _884 = (((_24.CB0.AmbientColor + (((_24.CB0.Lamp0Color * clamp(_796, 0.0, 1.0)) + (_24.CB0.Lamp1Color * max(-_796, 0.0))) * _1356)) + _1325.xyz) * vec4(_846.x, _846.y, _846.z, _767.w).xyz) + (_24.CB0.Lamp0Color * (((step(0.0, _796) * _1442) * _1356) * pow(clamp(dot(_791, normalize(_795 + normalize(IN_View_Depth.xyz))), 0.0, 1.0), _1446)));
    vec4 _886 = vec4(_884.x, _884.y, _884.z, _1512.w);
    _886.w = vec4(_846.x, _846.y, _846.z, _767.w).w;
    vec2 _909 = min(IN_Uv_EdgeDistance1.wz, IN_UvStuds_EdgeDistance2.wz);
    float _916 = min(_909.x, _909.y) / _978;
    vec3 _926 = _886.xyz * clamp((clamp((_978 * _24.CB0.OutlineBrightness_ShadowInfo.x) + _24.CB0.OutlineBrightness_ShadowInfo.y, 0.0, 1.0) * (1.5 - _916)) + _916, 0.0, 1.0);
    vec4 _928 = vec4(_926.x, _926.y, _926.z, _886.w);
    vec3 _938 = mix(_24.CB0.FogColor, _928.xyz, vec3(clamp(_1378.LightPosition_Fog.w, 0.0, 1.0)));
    _entryPointOutput = vec4(_938.x, _938.y, _938.z, _928.w);
}

