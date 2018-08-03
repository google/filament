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

vec2 warp_position()
{
    float vlod = dot(LODWeights, _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].LODs);
    vlod = all(equal(LODWeights, vec4(0.0))) ? _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.w : vlod;
    float floor_lod = floor(vlod);
    float fract_lod = vlod - floor_lod;
    uint ufloor_lod = uint(floor_lod);
    uvec4 uPosition = uvec4(Position);
    uvec2 mask = (uvec2(1u) << uvec2(ufloor_lod, ufloor_lod + 1u)) - uvec2(1u);
    uint _111;
    if (uPosition.x < 32u)
    {
        _111 = mask.x;
    }
    else
    {
        _111 = 0u;
    }
    uvec4 rounding;
    rounding.x = _111;
    uint _122;
    if (uPosition.y < 32u)
    {
        _122 = mask.x;
    }
    else
    {
        _122 = 0u;
    }
    rounding.y = _122;
    uint _133;
    if (uPosition.x < 32u)
    {
        _133 = mask.y;
    }
    else
    {
        _133 = 0u;
    }
    rounding.z = _133;
    uint _145;
    if (uPosition.y < 32u)
    {
        _145 = mask.y;
    }
    else
    {
        _145 = 0u;
    }
    rounding.w = _145;
    vec4 lower_upper_snapped = vec4((uPosition.xyxy + rounding) & (~mask).xxyy);
    return mix(lower_upper_snapped.xy, lower_upper_snapped.zw, vec2(fract_lod));
}

vec2 lod_factor(vec2 uv)
{
    float level = textureLod(TexLOD, uv, 0.0).x * 7.96875;
    float floor_level = floor(level);
    float fract_level = level - floor_level;
    return vec2(floor_level, fract_level);
}

void main()
{
    vec2 PatchPos = _53.Patches[(gl_InstanceID + SPIRV_Cross_BaseInstance)].Position.xz * _180.InvOceanSize_PatchScale.zw;
    vec2 WarpedPos = warp_position();
    vec2 VertexPos = PatchPos + WarpedPos;
    vec2 NormalizedPos = VertexPos * _180.InvOceanSize_PatchScale.xy;
    vec2 NormalizedTex = NormalizedPos * _180.NormalTexCoordScale.zw;
    vec2 param = NormalizedPos;
    vec2 lod = lod_factor(param);
    vec2 Offset = (_180.InvOceanSize_PatchScale.xy * exp2(lod.x)) * _180.NormalTexCoordScale.zw;
    vec3 Displacement = mix(textureLod(TexDisplacement, NormalizedTex + (Offset * 0.5), lod.x).yxz, textureLod(TexDisplacement, NormalizedTex + (Offset * 1.0), lod.x + 1.0).yxz, vec3(lod.y));
    vec3 WorldPos = vec3(NormalizedPos.x, 0.0, NormalizedPos.y) + Displacement;
    WorldPos *= _180.OceanScale.xyz;
    WorldPos += _180.OceanPosition.xyz;
    EyeVec = WorldPos - _273.g_CamPos.xyz;
    TexCoord = vec4(NormalizedTex, NormalizedTex * _180.NormalTexCoordScale.xy) + ((_180.InvOceanSize_PatchScale.xyxy * 0.5) * _180.NormalTexCoordScale.zwzw);
    gl_Position = (((_273.g_ViewProj_Row0 * WorldPos.x) + (_273.g_ViewProj_Row1 * WorldPos.y)) + (_273.g_ViewProj_Row2 * WorldPos.z)) + _273.g_ViewProj_Row3;
}

