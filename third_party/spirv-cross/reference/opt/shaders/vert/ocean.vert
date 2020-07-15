#version 310 es
#ifdef GL_ARB_shader_draw_parameters
#extension GL_ARB_shader_draw_parameters : enable
#endif

struct PatchData
{
    vec4 Position;
    vec4 LODs;
};

layout(binding = 0, std140) uniform Offsets
{
    PatchData Patches[256];
} _53;

layout(binding = 4, std140) uniform GlobalOcean
{
    vec4 OceanScale;
    vec4 OceanPosition;
    vec4 InvOceanSize_PatchScale;
    vec4 NormalTexCoordScale;
} _180;

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
} _273;

layout(binding = 1) uniform mediump sampler2D TexLOD;
layout(binding = 0) uniform mediump sampler2D TexDisplacement;

layout(location = 1) in vec4 LODWeights;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) in vec4 Position;
layout(location = 0) out vec3 EyeVec;
layout(location = 1) out vec4 TexCoord;

uvec4 _476;

void main()
{
    float _351 = all(equal(LODWeights, vec4(0.0))) ? _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.w : dot(LODWeights, _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].LODs);
    float _353 = floor(_351);
    uint _358 = uint(_353);
    uvec4 _360 = uvec4(Position);
    uvec2 _367 = (uvec2(1u) << uvec2(_358, _358 + 1u)) - uvec2(1u);
    bool _370 = _360.x < 32u;
    uint _467;
    if (_370)
    {
        _467 = _367.x;
    }
    else
    {
        _467 = 0u;
    }
    uvec4 _445 = _476;
    _445.x = _467;
    bool _380 = _360.y < 32u;
    uint _470;
    if (_380)
    {
        _470 = _367.x;
    }
    else
    {
        _470 = 0u;
    }
    uvec4 _449 = _445;
    _449.y = _470;
    uint _472;
    if (_370)
    {
        _472 = _367.y;
    }
    else
    {
        _472 = 0u;
    }
    uvec4 _453 = _449;
    _453.z = _472;
    uint _474;
    if (_380)
    {
        _474 = _367.y;
    }
    else
    {
        _474 = 0u;
    }
    uvec4 _457 = _453;
    _457.w = _474;
    vec4 _416 = vec4((_360.xyxy + _457) & (~_367).xxyy);
    vec2 _197 = ((_53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.xz * _180.InvOceanSize_PatchScale.zw) + mix(_416.xy, _416.zw, vec2(_351 - _353))) * _180.InvOceanSize_PatchScale.xy;
    vec2 _204 = _197 * _180.NormalTexCoordScale.zw;
    mediump float _433 = textureLod(TexLOD, _197, 0.0).x * 7.96875;
    float _435 = floor(_433);
    vec2 _220 = (_180.InvOceanSize_PatchScale.xy * exp2(_435)) * _180.NormalTexCoordScale.zw;
    vec3 _267 = ((vec3(_197.x, 0.0, _197.y) + mix(textureLod(TexDisplacement, _204 + (_220 * 0.5), _435).yxz, textureLod(TexDisplacement, _204 + (_220 * 1.0), _435 + 1.0).yxz, vec3(_433 - _435))) * _180.OceanScale.xyz) + _180.OceanPosition.xyz;
    EyeVec = _267 - _273.g_CamPos.xyz;
    TexCoord = vec4(_204, _204 * _180.NormalTexCoordScale.xy) + ((_180.InvOceanSize_PatchScale.xyxy * 0.5) * _180.NormalTexCoordScale.zwzw);
    gl_Position = (((_273.g_ViewProj_Row0 * _267.x) + (_273.g_ViewProj_Row1 * _267.y)) + (_273.g_ViewProj_Row2 * _267.z)) + _273.g_ViewProj_Row3;
}

