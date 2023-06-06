#version 310 es
#extension GL_EXT_tessellation_shader : require
precision highp int;

layout(cw, quads, fractional_even_spacing) in;

layout(location = 0) patch in vec2 vOutPatchPosBase;
layout(location = 1) patch in vec4 vPatchLods;

layout(binding = 1, std140) uniform UBO
{
    mat4 uMVP;
    vec4 uScale;
    vec2 uInvScale;
    vec3 uCamPos;
    vec2 uPatchSize;
    vec2 uInvHeightmapSize;
};
layout(binding = 0) uniform mediump sampler2D uHeightmapDisplacement;

layout(location = 0) highp out vec3 vWorld;
layout(location = 1) highp out vec4 vGradNormalTex;

vec2 lerp_vertex(vec2 tess_coord)
{
    return vOutPatchPosBase + tess_coord * uPatchSize;
}

mediump vec2 lod_factor(vec2 tess_coord)
{
    mediump vec2 x = mix(vPatchLods.yx, vPatchLods.zw, tess_coord.x);
    mediump float level = mix(x.x, x.y, tess_coord.y);
    mediump float floor_level = floor(level);
    mediump float fract_level = level - floor_level;
    return vec2(floor_level, fract_level);
}

mediump vec3 sample_height_displacement(vec2 uv, vec2 off, mediump vec2 lod)
{
    return mix(
            textureLod(uHeightmapDisplacement, uv + 0.5 * off, lod.x).xyz,
            textureLod(uHeightmapDisplacement, uv + 1.0 * off, lod.x + 1.0).xyz,
            lod.y);
}

void main()
{
    vec2 tess_coord = gl_TessCoord.xy;
    vec2 pos = lerp_vertex(tess_coord);
    mediump vec2 lod = lod_factor(tess_coord);

    vec2 tex = pos * uInvHeightmapSize.xy;
    pos *= uScale.xy;

    mediump float delta_mod = exp2(lod.x);
    vec2 off = uInvHeightmapSize.xy * delta_mod;

    vGradNormalTex = vec4(tex + 0.5 * uInvHeightmapSize.xy, tex * uScale.zw);
    vec3 height_displacement = sample_height_displacement(tex, off, lod);

    pos += height_displacement.yz;
    vWorld = vec3(pos.x, height_displacement.x, pos.y);
    gl_Position = uMVP * vec4(vWorld, 1.0);
}

