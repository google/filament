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

layout(binding = 0, std140) uniform CB0
{
    Globals CB0;
} _19;

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

VertexOutput _121;
SurfaceInput _122;
vec2 _123;
vec4 _124;
Surface _125;
vec4 _192;
vec4 _219;
vec4 _297;

void main()
{
    VertexOutput _128 = _121;
    _128.HPosition = gl_FragCoord;
    VertexOutput _130 = _128;
    _130.Uv_EdgeDistance1 = IN_Uv_EdgeDistance1;
    VertexOutput _132 = _130;
    _132.UvStuds_EdgeDistance2 = IN_UvStuds_EdgeDistance2;
    VertexOutput _134 = _132;
    _134.Color = IN_Color;
    VertexOutput _136 = _134;
    _136.LightPosition_Fog = IN_LightPosition_Fog;
    VertexOutput _138 = _136;
    _138.View_Depth = IN_View_Depth;
    VertexOutput _140 = _138;
    _140.Normal_SpecPower = IN_Normal_SpecPower;
    VertexOutput _142 = _140;
    _142.Tangent = IN_Tangent;
    VertexOutput _144 = _142;
    _144.PosLightSpace_Reflectance = IN_PosLightSpace_Reflectance;
    VertexOutput _146 = _144;
    _146.studIndex = IN_studIndex;
    SurfaceInput _147 = _122;
    _147.Color = IN_Color;
    SurfaceInput _149 = _147;
    _149.Uv = IN_Uv_EdgeDistance1.xy;
    SurfaceInput _151 = _149;
    _151.UvStuds = IN_UvStuds_EdgeDistance2.xy;
    SurfaceInput _156 = _151;
    _156.UvStuds.y = (fract(_151.UvStuds.y) + IN_studIndex) * 0.25;
    float _163 = _146.View_Depth.w * _19.CB0.RefractionBias_FadeDistance_GlowFactor.y;
    float _165 = clamp(1.0 - _163, 0.0, 1.0);
    vec2 _166 = IN_Uv_EdgeDistance1.xy * 1.0;
    bool _173;
    vec4 _193;
    do
    {
        _173 = 0.0 == 0.0;
        if (_173)
        {
            _193 = texture(SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler, _166);
            break;
        }
        else
        {
            float _180 = 1.0 / (1.0 - 0.0);
            _193 = mix(texture(SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler, _166 * 0.25), texture(SPIRV_Cross_CombinedDiffuseMapTextureDiffuseMapSampler, _166), vec4(clamp((clamp(1.0 - (_146.View_Depth.w * 0.00333332992158830165863037109375), 0.0, 1.0) * _180) - (0.0 * _180), 0.0, 1.0)));
            break;
        }
        _193 = _192;
        break;
    } while (false);
    vec4 _194 = _193 * 1.0;
    vec4 _220;
    do
    {
        if (_173)
        {
            _220 = texture(SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler, _166);
            break;
        }
        else
        {
            float _207 = 1.0 / (1.0 - 0.0);
            _220 = mix(texture(SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler, _166 * 0.25), texture(SPIRV_Cross_CombinedNormalMapTextureNormalMapSampler, _166), vec4(clamp((_165 * _207) - (0.0 * _207), 0.0, 1.0)));
            break;
        }
        _220 = _219;
        break;
    } while (false);
    vec2 _223 = vec2(1.0);
    vec2 _224 = (_220.wy * 2.0) - _223;
    vec3 _232 = vec3(_224, sqrt(clamp(1.0 + dot(-_224, _224), 0.0, 1.0)));
    vec2 _240 = (texture(SPIRV_Cross_CombinedNormalDetailMapTextureNormalDetailMapSampler, _166 * 0.0).wy * 2.0) - _223;
    vec2 _252 = _232.xy + (vec3(_240, sqrt(clamp(1.0 + dot(-_240, _240), 0.0, 1.0))).xy * 0.0);
    vec3 _253 = vec3(_252.x, _252.y, _232.z);
    vec2 _255 = _253.xy * _165;
    vec3 _256 = vec3(_255.x, _255.y, _253.z);
    vec3 _271 = ((IN_Color.xyz * _194.xyz) * (1.0 + (_256.x * 0.300000011920928955078125))) * (texture(SPIRV_Cross_CombinedStudsMapTextureStudsMapSampler, _156.UvStuds).x * 2.0);
    vec4 _298;
    do
    {
        if (0.75 == 0.0)
        {
            _298 = texture(SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler, _166);
            break;
        }
        else
        {
            float _285 = 1.0 / (1.0 - 0.75);
            _298 = mix(texture(SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler, _166 * 0.25), texture(SPIRV_Cross_CombinedSpecularMapTextureSpecularMapSampler, _166), vec4(clamp((_165 * _285) - (0.75 * _285), 0.0, 1.0)));
            break;
        }
        _298 = _297;
        break;
    } while (false);
    vec2 _303 = mix(vec2(0.800000011920928955078125, 120.0), (_298.xy * vec2(2.0, 256.0)) + vec2(0.0, 0.00999999977648258209228515625), vec2(_165));
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
    vec4 _318 = vec4(_271, _146.Color.w);
    vec3 _329 = normalize(((IN_Tangent * _313.normal.x) + (cross(IN_Normal_SpecPower.xyz, IN_Tangent) * _313.normal.y)) + (IN_Normal_SpecPower.xyz * _313.normal.z));
    vec3 _332 = -_19.CB0.Lamp0Dir;
    float _333 = dot(_329, _332);
    float _357 = clamp(dot(step(_19.CB0.LightConfig3.xyz, abs(IN_LightPosition_Fog.xyz - _19.CB0.LightConfig2.xyz)), vec3(1.0)), 0.0, 1.0);
    vec4 _368 = mix(texture(SPIRV_Cross_CombinedLightMapTextureLightMapSampler, IN_LightPosition_Fog.xyz.yzx - (IN_LightPosition_Fog.xyz.yzx * _357)), _19.CB0.LightBorder, vec4(_357));
    vec2 _376 = texture(SPIRV_Cross_CombinedShadowMapTextureShadowMapSampler, IN_PosLightSpace_Reflectance.xyz.xy).xy;
    float _392 = (1.0 - (((step(_376.x, IN_PosLightSpace_Reflectance.xyz.z) * clamp(9.0 - (20.0 * abs(IN_PosLightSpace_Reflectance.xyz.z - 0.5)), 0.0, 1.0)) * _376.y) * _19.CB0.OutlineBrightness_ShadowInfo.w)) * _368.w;
    vec3 _403 = mix(_318.xyz, texture(SPIRV_Cross_CombinedEnvironmentMapTextureEnvironmentMapSampler, reflect(-IN_View_Depth.xyz, _329)).xyz, vec3(_312));
    vec4 _404 = vec4(_403.x, _403.y, _403.z, _318.w);
    vec3 _422 = (((_19.CB0.AmbientColor + (((_19.CB0.Lamp0Color * clamp(_333, 0.0, 1.0)) + (_19.CB0.Lamp1Color * max(-_333, 0.0))) * _392)) + _368.xyz) * _404.xyz) + (_19.CB0.Lamp0Color * (((step(0.0, _333) * _306) * _392) * pow(clamp(dot(_329, normalize(_332 + normalize(IN_View_Depth.xyz))), 0.0, 1.0), _308)));
    vec4 _425 = vec4(_422.x, _422.y, _422.z, _124.w);
    _425.w = _404.w;
    vec2 _435 = min(IN_Uv_EdgeDistance1.wz, IN_UvStuds_EdgeDistance2.wz);
    float _439 = min(_435.x, _435.y) / _163;
    vec3 _445 = _425.xyz * clamp((clamp((_163 * _19.CB0.OutlineBrightness_ShadowInfo.x) + _19.CB0.OutlineBrightness_ShadowInfo.y, 0.0, 1.0) * (1.5 - _439)) + _439, 0.0, 1.0);
    vec4 _446 = vec4(_445.x, _445.y, _445.z, _425.w);
    vec3 _453 = mix(_19.CB0.FogColor, _446.xyz, vec3(clamp(_146.LightPosition_Fog.w, 0.0, 1.0)));
    _entryPointOutput = vec4(_453.x, _453.y, _453.z, _446.w);
}

