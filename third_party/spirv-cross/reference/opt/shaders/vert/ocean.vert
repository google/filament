#version 310 es

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
uniform int SPIRV_Cross_BaseInstance;
layout(location = 0) in vec4 Position;
layout(location = 0) out vec3 EyeVec;
layout(location = 1) out vec4 TexCoord;

uvec4 _474;

void main()
{
    float _350 = all(equal(LODWeights, vec4(0.0))) ? _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.w : dot(LODWeights, _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].LODs);
    float _352 = floor(_350);
    uint _357 = uint(_352);
    uvec4 _359 = uvec4(Position);
    uvec2 _366 = (uvec2(1u) << uvec2(_357, _357 + 1u)) - uvec2(1u);
    bool _369 = _359.x < 32u;
    uint _465;
    if (_369)
    {
        _465 = _366.x;
    }
    else
    {
        _465 = 0u;
    }
    uvec4 _443 = _474;
    _443.x = _465;
    bool _379 = _359.y < 32u;
    uint _468;
    if (_379)
    {
        _468 = _366.x;
    }
    else
    {
        _468 = 0u;
    }
    uvec4 _447 = _443;
    _447.y = _468;
    uint _470;
    if (_369)
    {
        _470 = _366.y;
    }
    else
    {
        _470 = 0u;
    }
    uvec4 _451 = _447;
    _451.z = _470;
    uint _472;
    if (_379)
    {
        _472 = _366.y;
    }
    else
    {
        _472 = 0u;
    }
    uvec4 _455 = _451;
    _455.w = _472;
    vec4 _415 = vec4((_359.xyxy + _455) & (~_366).xxyy);
    vec2 _197 = ((_53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.xz * _180.InvOceanSize_PatchScale.zw) + mix(_415.xy, _415.zw, vec2(_350 - _352))) * _180.InvOceanSize_PatchScale.xy;
    vec2 _204 = _197 * _180.NormalTexCoordScale.zw;
    mediump float _431 = textureLod(TexLOD, _197, 0.0).x * 7.96875;
    float _433 = floor(_431);
    vec2 _220 = (_180.InvOceanSize_PatchScale.xy * exp2(_433)) * _180.NormalTexCoordScale.zw;
    vec3 _267 = ((vec3(_197.x, 0.0, _197.y) + mix(textureLod(TexDisplacement, _204 + (_220 * 0.5), _433).yxz, textureLod(TexDisplacement, _204 + (_220 * 1.0), _433 + 1.0).yxz, vec3(_431 - _433))) * _180.OceanScale.xyz) + _180.OceanPosition.xyz;
    EyeVec = _267 - _273.g_CamPos.xyz;
    TexCoord = vec4(_204, _204 * _180.NormalTexCoordScale.xy) + ((_180.InvOceanSize_PatchScale.xyxy * 0.5) * _180.NormalTexCoordScale.zwzw);
    gl_Position = (((_273.g_ViewProj_Row0 * _267.x) + (_273.g_ViewProj_Row1 * _267.y)) + (_273.g_ViewProj_Row2 * _267.z)) + _273.g_ViewProj_Row3;
}

