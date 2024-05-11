#version 310 es
#extension GL_EXT_tessellation_shader : require

layout(vertices = 1) out;
layout(location = 0) in vec2 vPatchPosBase[];

layout(std140) uniform UBO
{
    vec4 uScale;
    highp vec3 uCamPos;
    vec2 uPatchSize;
    vec2 uMaxTessLevel;
    float uDistanceMod;
    vec4 uFrustum[6];
};

layout(location = 1) patch out vec2 vOutPatchPosBase;
layout(location = 2) patch out vec4 vPatchLods;

float lod_factor(vec2 pos_)
{
    vec2 pos = pos_ * uScale.xy;
    vec3 dist_to_cam = uCamPos - vec3(pos.x, 0.0, pos.y);
    float level = log2((length(dist_to_cam) + 0.0001) * uDistanceMod);
    return clamp(level, 0.0, uMaxTessLevel.x);
}

float tess_level(float lod)
{
    return uMaxTessLevel.y * exp2(-lod);
}

vec4 tess_level(vec4 lod)
{
    return uMaxTessLevel.y * exp2(-lod);
}

// Guard band for vertex displacement.
#define GUARD_BAND 10.0
bool frustum_cull(vec2 p0)
{
    vec2 min_xz = (p0 - GUARD_BAND) * uScale.xy;
    vec2 max_xz = (p0 + uPatchSize + GUARD_BAND) * uScale.xy;

    vec3 bb_min = vec3(min_xz.x, -GUARD_BAND, min_xz.y);
    vec3 bb_max = vec3(max_xz.x, +GUARD_BAND, max_xz.y);
    vec3 center = 0.5 * (bb_min + bb_max);
    float radius = 0.5 * length(bb_max - bb_min);

    vec3 f0 = vec3(
        dot(uFrustum[0], vec4(center, 1.0)),
        dot(uFrustum[1], vec4(center, 1.0)),
        dot(uFrustum[2], vec4(center, 1.0)));

    vec3 f1 = vec3(
        dot(uFrustum[3], vec4(center, 1.0)),
        dot(uFrustum[4], vec4(center, 1.0)),
        dot(uFrustum[5], vec4(center, 1.0)));

    return !(any(lessThanEqual(f0, vec3(-radius))) || any(lessThanEqual(f1, vec3(-radius))));
}

void compute_tess_levels(vec2 p0)
{
    vOutPatchPosBase = p0;

    float l00 = lod_factor(p0 + vec2(-0.5, -0.5) * uPatchSize);
    float l10 = lod_factor(p0 + vec2(+0.5, -0.5) * uPatchSize);
    float l20 = lod_factor(p0 + vec2(+1.5, -0.5) * uPatchSize);
    float l01 = lod_factor(p0 + vec2(-0.5, +0.5) * uPatchSize);
    float l11 = lod_factor(p0 + vec2(+0.5, +0.5) * uPatchSize);
    float l21 = lod_factor(p0 + vec2(+1.5, +0.5) * uPatchSize);
    float l02 = lod_factor(p0 + vec2(-0.5, +1.5) * uPatchSize);
    float l12 = lod_factor(p0 + vec2(+0.5, +1.5) * uPatchSize);
    float l22 = lod_factor(p0 + vec2(+1.5, +1.5) * uPatchSize);

    vec4 lods = vec4(
        dot(vec4(l01, l11, l02, l12), vec4(0.25)),
        dot(vec4(l00, l10, l01, l11), vec4(0.25)),
        dot(vec4(l10, l20, l11, l21), vec4(0.25)),
        dot(vec4(l11, l21, l12, l22), vec4(0.25)));

    vPatchLods = lods;

    vec4 outer_lods = min(lods.xyzw, lods.yzwx);
    vec4 levels = tess_level(outer_lods);
    gl_TessLevelOuter[0] = levels.x;
    gl_TessLevelOuter[1] = levels.y;
    gl_TessLevelOuter[2] = levels.z;
    gl_TessLevelOuter[3] = levels.w;

    float min_lod = min(min(lods.x, lods.y), min(lods.z, lods.w));
    float inner = tess_level(min(min_lod, l11));
    gl_TessLevelInner[0] = inner;
    gl_TessLevelInner[1] = inner;
}

void main()
{
    vec2 p0 = vPatchPosBase[0];
    if (!frustum_cull(p0))
    {
        gl_TessLevelOuter[0] = -1.0;
        gl_TessLevelOuter[1] = -1.0;
        gl_TessLevelOuter[2] = -1.0;
        gl_TessLevelOuter[3] = -1.0;
        gl_TessLevelInner[0] = -1.0;
        gl_TessLevelInner[1] = -1.0;
    }
    else
    {
        compute_tess_levels(p0);
    }
}

