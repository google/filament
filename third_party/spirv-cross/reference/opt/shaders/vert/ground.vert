#version 310 es
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

struct PatchData
{
    vec4 Position;
    vec4 LODs;
};

layout(binding = 0, std140) uniform PerPatch
{
    PatchData Patches[256];
} _53;

layout(binding = 2, std140) uniform GlobalGround
{
    vec4 GroundScale;
    vec4 GroundPosition;
    vec4 InvGroundSize_PatchScale;
} _156;

layout(binding = 0, std140) uniform GlobalVSData
{
    vec4 g_ViewProj_Row0;
    vec4 g_ViewProj_Row1;
    vec4 g_ViewProj_Row2;
    vec4 g_ViewProj_Row3;
    vec4 g_CamPos;
    vec4 g_CamRight;
    vec4 g_CamUp;
    vec4 g_CamFront;
    vec4 g_SunDir;
    vec4 g_SunColor;
    vec4 g_TimeParams;
    vec4 g_ResolutionParams;
    vec4 g_CamAxisRight;
    vec4 g_FogColor_Distance;
    vec4 g_ShadowVP_Row0;
    vec4 g_ShadowVP_Row1;
    vec4 g_ShadowVP_Row2;
    vec4 g_ShadowVP_Row3;
} _236;

layout(binding = 1) uniform mediump sampler2D TexLOD;
layout(binding = 0) uniform mediump sampler2D TexHeightmap;

layout(location = 1) in vec4 LODWeights;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) in vec2 Position;
layout(location = 1) out vec3 EyeVec;
layout(location = 0) out vec2 TexCoord;

void main()
{
    float _301 = all(equal(LODWeights, vec4(0.0))) ? _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.w : dot(LODWeights, _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].LODs);
    float _303 = floor(_301);
    uint _308 = uint(_303);
    uvec2 _310 = uvec2(Position);
    uvec2 _317 = (uvec2(1u) << uvec2(_308, _308 + 1u)) - uvec2(1u);
    uint _384;
    if (_310.x < 32u)
    {
        _384 = _317.x;
    }
    else
    {
        _384 = 0u;
    }
    uint _385;
    if (_310.y < 32u)
    {
        _385 = _317.y;
    }
    else
    {
        _385 = 0u;
    }
    vec4 _345 = vec4((_310 + uvec2(_384, _385)).xyxy & (~_317).xxyy);
    vec2 _167 = _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.xz * _156.InvGroundSize_PatchScale.zw + mix(_345.xy, _345.zw, vec2(_301 - _303));
    vec2 _173 = _167 * _156.InvGroundSize_PatchScale.xy;
    mediump vec4 _360 = textureLod(TexLOD, _173, 0.0);
    mediump float _361 = _360.x;
    mediump float _362 = _361 * 7.96875;
    float hp_copy_362 = _362;
    float _364 = floor(hp_copy_362);
    vec2 _185 = _156.InvGroundSize_PatchScale.xy * exp2(_364);
    vec3 _230 = vec3(_173.x, mix(textureLod(TexHeightmap, _167 * _156.InvGroundSize_PatchScale.xy + (_185 * 0.5), _364).x, textureLod(TexHeightmap, _167 * _156.InvGroundSize_PatchScale.xy + (_185 * 1.0), _364 + 1.0).x, _361 * 7.96875 + (-_364)), _173.y) * _156.GroundScale.xyz + _156.GroundPosition.xyz;
    EyeVec = _230 - _236.g_CamPos.xyz;
    TexCoord = _167 * _156.InvGroundSize_PatchScale.xy + (_156.InvGroundSize_PatchScale.xy * 0.5);
    gl_Position = (((_236.g_ViewProj_Row0 * _230.x) + (_236.g_ViewProj_Row1 * _230.y)) + (_236.g_ViewProj_Row2 * _230.z)) + _236.g_ViewProj_Row3;
}

