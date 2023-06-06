#version 310 es
#extension GL_EXT_tessellation_shader : require
layout(vertices = 1) out;

layout(binding = 0, std140) uniform UBO
{
    vec4 uScale;
    vec3 uCamPos;
    vec2 uPatchSize;
    vec2 uMaxTessLevel;
    float uDistanceMod;
    vec4 uFrustum[6];
} _41;

layout(location = 1) patch out vec2 vOutPatchPosBase;
layout(location = 2) patch out vec4 vPatchLods;
layout(location = 0) in vec2 vPatchPosBase[];

bool frustum_cull(vec2 p0)
{
    vec2 min_xz = (p0 - vec2(10.0)) * _41.uScale.xy;
    vec2 max_xz = ((p0 + _41.uPatchSize) + vec2(10.0)) * _41.uScale.xy;
    vec3 bb_min = vec3(min_xz.x, -10.0, min_xz.y);
    vec3 bb_max = vec3(max_xz.x, 10.0, max_xz.y);
    vec3 center = (bb_min + bb_max) * 0.5;
    float radius = 0.5 * length(bb_max - bb_min);
    vec3 f0 = vec3(dot(_41.uFrustum[0], vec4(center, 1.0)), dot(_41.uFrustum[1], vec4(center, 1.0)), dot(_41.uFrustum[2], vec4(center, 1.0)));
    vec3 f1 = vec3(dot(_41.uFrustum[3], vec4(center, 1.0)), dot(_41.uFrustum[4], vec4(center, 1.0)), dot(_41.uFrustum[5], vec4(center, 1.0)));
    bool _205 = any(lessThanEqual(f0, vec3(-radius)));
    bool _215;
    if (!_205)
    {
        _215 = any(lessThanEqual(f1, vec3(-radius)));
    }
    else
    {
        _215 = _205;
    }
    return !_215;
}

float lod_factor(vec2 pos_)
{
    vec2 pos = pos_ * _41.uScale.xy;
    vec3 dist_to_cam = _41.uCamPos - vec3(pos.x, 0.0, pos.y);
    float level = log2((length(dist_to_cam) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod);
    return clamp(level, 0.0, _41.uMaxTessLevel.x);
}

vec4 tess_level(vec4 lod)
{
    return exp2(-lod) * _41.uMaxTessLevel.y;
}

float tess_level(float lod)
{
    return _41.uMaxTessLevel.y * exp2(-lod);
}

void compute_tess_levels(vec2 p0)
{
    vOutPatchPosBase = p0;
    vec2 param = p0 + (vec2(-0.5) * _41.uPatchSize);
    float l00 = lod_factor(param);
    vec2 param_1 = p0 + (vec2(0.5, -0.5) * _41.uPatchSize);
    float l10 = lod_factor(param_1);
    vec2 param_2 = p0 + (vec2(1.5, -0.5) * _41.uPatchSize);
    float l20 = lod_factor(param_2);
    vec2 param_3 = p0 + (vec2(-0.5, 0.5) * _41.uPatchSize);
    float l01 = lod_factor(param_3);
    vec2 param_4 = p0 + (vec2(0.5) * _41.uPatchSize);
    float l11 = lod_factor(param_4);
    vec2 param_5 = p0 + (vec2(1.5, 0.5) * _41.uPatchSize);
    float l21 = lod_factor(param_5);
    vec2 param_6 = p0 + (vec2(-0.5, 1.5) * _41.uPatchSize);
    float l02 = lod_factor(param_6);
    vec2 param_7 = p0 + (vec2(0.5, 1.5) * _41.uPatchSize);
    float l12 = lod_factor(param_7);
    vec2 param_8 = p0 + (vec2(1.5) * _41.uPatchSize);
    float l22 = lod_factor(param_8);
    vec4 lods = vec4(dot(vec4(l01, l11, l02, l12), vec4(0.25)), dot(vec4(l00, l10, l01, l11), vec4(0.25)), dot(vec4(l10, l20, l11, l21), vec4(0.25)), dot(vec4(l11, l21, l12, l22), vec4(0.25)));
    vPatchLods = lods;
    vec4 outer_lods = min(lods, lods.yzwx);
    vec4 param_9 = outer_lods;
    vec4 levels = tess_level(param_9);
    gl_TessLevelOuter[0] = levels.x;
    gl_TessLevelOuter[1] = levels.y;
    gl_TessLevelOuter[2] = levels.z;
    gl_TessLevelOuter[3] = levels.w;
    float min_lod = min(min(lods.x, lods.y), min(lods.z, lods.w));
    float param_10 = min(min_lod, l11);
    float inner = tess_level(param_10);
    gl_TessLevelInner[0] = inner;
    gl_TessLevelInner[1] = inner;
}

void main()
{
    vec2 p0 = vPatchPosBase[0];
    vec2 param = p0;
    if (!frustum_cull(param))
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
        vec2 param_1 = p0;
        compute_tess_levels(param_1);
    }
}

