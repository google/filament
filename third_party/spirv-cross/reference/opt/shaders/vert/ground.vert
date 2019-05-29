#version 310 es

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
uniform int SPIRV_Cross_BaseInstance;
layout(location = 0) in vec2 Position;
layout(location = 1) out vec3 EyeVec;
layout(location = 0) out vec2 TexCoord;

void main()
{
    float _300 = all(equal(LODWeights, vec4(0.0))) ? _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.w : dot(LODWeights, _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].LODs);
    float _302 = floor(_300);
    uint _307 = uint(_302);
    uvec2 _309 = uvec2(Position);
    uvec2 _316 = (uvec2(1u) << uvec2(_307, _307 + 1u)) - uvec2(1u);
    uint _382;
    if (_309.x < 32u)
    {
        _382 = _316.x;
    }
    else
    {
        _382 = 0u;
    }
    uint _383;
    if (_309.y < 32u)
    {
        _383 = _316.y;
    }
    else
    {
        _383 = 0u;
    }
    vec4 _344 = vec4((_309 + uvec2(_382, _383)).xyxy & (~_316).xxyy);
    vec2 _173 = ((_53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.xz * _156.InvGroundSize_PatchScale.zw) + mix(_344.xy, _344.zw, vec2(_300 - _302))) * _156.InvGroundSize_PatchScale.xy;
    mediump float _360 = textureLod(TexLOD, _173, 0.0).x * 7.96875;
    float _362 = floor(_360);
    vec2 _185 = _156.InvGroundSize_PatchScale.xy * exp2(_362);
    vec3 _230 = (vec3(_173.x, mix(textureLod(TexHeightmap, _173 + (_185 * 0.5), _362).x, textureLod(TexHeightmap, _173 + (_185 * 1.0), _362 + 1.0).x, _360 - _362), _173.y) * _156.GroundScale.xyz) + _156.GroundPosition.xyz;
    EyeVec = _230 - _236.g_CamPos.xyz;
    TexCoord = _173 + (_156.InvGroundSize_PatchScale.xy * 0.5);
    gl_Position = (((_236.g_ViewProj_Row0 * _230.x) + (_236.g_ViewProj_Row1 * _230.y)) + (_236.g_ViewProj_Row2 * _230.z)) + _236.g_ViewProj_Row3;
}

