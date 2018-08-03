#version 310 es
#extension GL_EXT_tessellation_shader : require
layout(vertices = 1) out;

layout(std140) uniform UBO
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

void main()
{
    vec2 _430 = (vPatchPosBase[0] - vec2(10.0)) * _41.uScale.xy;
    vec2 _440 = ((vPatchPosBase[0] + _41.uPatchSize) + vec2(10.0)) * _41.uScale.xy;
    vec3 _445 = vec3(_430.x, -10.0, _430.y);
    vec3 _450 = vec3(_440.x, 10.0, _440.y);
    vec4 _466 = vec4((_445 + _450) * 0.5, 1.0);
    vec3 _513 = vec3(length(_450 - _445) * (-0.5));
    bool _515 = any(lessThanEqual(vec3(dot(_41.uFrustum[0], _466), dot(_41.uFrustum[1], _466), dot(_41.uFrustum[2], _466)), _513));
    bool _525;
    if (!_515)
    {
        _525 = any(lessThanEqual(vec3(dot(_41.uFrustum[3], _466), dot(_41.uFrustum[4], _466), dot(_41.uFrustum[5], _466)), _513));
    }
    else
    {
        _525 = _515;
    }
    if (!(!_525))
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
        vOutPatchPosBase = vPatchPosBase[0];
        vec2 _678 = (vPatchPosBase[0] + (vec2(-0.5) * _41.uPatchSize)) * _41.uScale.xy;
        vec2 _706 = (vPatchPosBase[0] + (vec2(0.5, -0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _725 = clamp(log2((length(_41.uCamPos - vec3(_706.x, 0.0, _706.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _734 = (vPatchPosBase[0] + (vec2(1.5, -0.5) * _41.uPatchSize)) * _41.uScale.xy;
        vec2 _762 = (vPatchPosBase[0] + (vec2(-0.5, 0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _781 = clamp(log2((length(_41.uCamPos - vec3(_762.x, 0.0, _762.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _790 = (vPatchPosBase[0] + (vec2(0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _809 = clamp(log2((length(_41.uCamPos - vec3(_790.x, 0.0, _790.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _818 = (vPatchPosBase[0] + (vec2(1.5, 0.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _837 = clamp(log2((length(_41.uCamPos - vec3(_818.x, 0.0, _818.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _846 = (vPatchPosBase[0] + (vec2(-0.5, 1.5) * _41.uPatchSize)) * _41.uScale.xy;
        vec2 _874 = (vPatchPosBase[0] + (vec2(0.5, 1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _893 = clamp(log2((length(_41.uCamPos - vec3(_874.x, 0.0, _874.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _902 = (vPatchPosBase[0] + (vec2(1.5) * _41.uPatchSize)) * _41.uScale.xy;
        float _612 = dot(vec4(_781, _809, clamp(log2((length(_41.uCamPos - vec3(_846.x, 0.0, _846.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _893), vec4(0.25));
        float _618 = dot(vec4(clamp(log2((length(_41.uCamPos - vec3(_678.x, 0.0, _678.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _725, _781, _809), vec4(0.25));
        float _624 = dot(vec4(_725, clamp(log2((length(_41.uCamPos - vec3(_734.x, 0.0, _734.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _809, _837), vec4(0.25));
        float _630 = dot(vec4(_809, _837, _893, clamp(log2((length(_41.uCamPos - vec3(_902.x, 0.0, _902.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x)), vec4(0.25));
        vec4 _631 = vec4(_612, _618, _624, _630);
        vPatchLods = _631;
        vec4 _928 = exp2(-min(_631, _631.yzwx)) * _41.uMaxTessLevel.y;
        gl_TessLevelOuter[0] = _928.x;
        gl_TessLevelOuter[1] = _928.y;
        gl_TessLevelOuter[2] = _928.z;
        gl_TessLevelOuter[3] = _928.w;
        float _935 = _41.uMaxTessLevel.y * exp2(-min(min(min(_612, _618), min(_624, _630)), _809));
        gl_TessLevelInner[0] = _935;
        gl_TessLevelInner[1] = _935;
    }
}

