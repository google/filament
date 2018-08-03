#version 310 es
#extension GL_EXT_tessellation_shader : require
layout(quads, cw, fractional_even_spacing) in;

layout(binding = 1, std140) uniform UBO
{
    mat4 uMVP;
    vec4 uScale;
    vec2 uInvScale;
    vec3 uCamPos;
    vec2 uPatchSize;
    vec2 uInvHeightmapSize;
} _31;

layout(binding = 0) uniform mediump sampler2D uHeightmapDisplacement;

layout(location = 0) patch in vec2 vOutPatchPosBase;
layout(location = 1) patch in vec4 vPatchLods;
layout(location = 1) out vec4 vGradNormalTex;
layout(location = 0) out vec3 vWorld;

vec2 lerp_vertex(vec2 tess_coord)
{
    return vOutPatchPosBase + (tess_coord * _31.uPatchSize);
}

mediump vec2 lod_factor(vec2 tess_coord)
{
    mediump vec2 x = mix(vPatchLods.yx, vPatchLods.zw, vec2(tess_coord.x));
    mediump float level = mix(x.x, x.y, tess_coord.y);
    mediump float floor_level = floor(level);
    mediump float fract_level = level - floor_level;
    return vec2(floor_level, fract_level);
}

mediump vec3 sample_height_displacement(vec2 uv, vec2 off, mediump vec2 lod)
{
    return mix(textureLod(uHeightmapDisplacement, uv + (off * 0.5), lod.x).xyz, textureLod(uHeightmapDisplacement, uv + (off * 1.0), lod.x + 1.0).xyz, vec3(lod.y));
}

void main()
{
    vec2 tess_coord = gl_TessCoord.xy;
    vec2 param = tess_coord;
    vec2 pos = lerp_vertex(param);
    vec2 param_1 = tess_coord;
    mediump vec2 lod = lod_factor(param_1);
    vec2 tex = pos * _31.uInvHeightmapSize;
    pos *= _31.uScale.xy;
    mediump float delta_mod = exp2(lod.x);
    vec2 off = _31.uInvHeightmapSize * delta_mod;
    vGradNormalTex = vec4(tex + (_31.uInvHeightmapSize * 0.5), tex * _31.uScale.zw);
    vec2 param_2 = tex;
    vec2 param_3 = off;
    vec2 param_4 = lod;
    vec3 height_displacement = sample_height_displacement(param_2, param_3, param_4);
    pos += height_displacement.yz;
    vWorld = vec3(pos.x, height_displacement.x, pos.y);
    gl_Position = _31.uMVP * vec4(vWorld, 1.0);
}

