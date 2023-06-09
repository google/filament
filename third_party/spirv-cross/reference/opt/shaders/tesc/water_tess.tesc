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

void main()
{
    vec2 _431 = (vPatchPosBase[0] - vec2(10.0)) * _41.uScale.xy;
    vec2 _441 = ((vPatchPosBase[0] + _41.uPatchSize) + vec2(10.0)) * _41.uScale.xy;
    vec3 _446 = vec3(_431.x, -10.0, _431.y);
    vec3 _451 = vec3(_441.x, 10.0, _441.y);
    vec4 _467 = vec4((_446 + _451) * 0.5, 1.0);
    vec3 _514 = vec3(length(_451 - _446) * (-0.5));
    bool _516 = any(lessThanEqual(vec3(dot(_41.uFrustum[0], _467), dot(_41.uFrustum[1], _467), dot(_41.uFrustum[2], _467)), _514));
    bool _526;
    if (!_516)
    {
        _526 = any(lessThanEqual(vec3(dot(_41.uFrustum[3], _467), dot(_41.uFrustum[4], _467), dot(_41.uFrustum[5], _467)), _514));
    }
    else
    {
        _526 = _516;
    }
    if (!(!_526))
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
        vec2 _681 = (vec2(-0.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        vec2 _710 = (vec2(0.5, -0.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        float _729 = clamp(log2((length(_41.uCamPos - vec3(_710.x, 0.0, _710.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _739 = (vec2(1.5, -0.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        vec2 _768 = (vec2(-0.5, 0.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        float _787 = clamp(log2((length(_41.uCamPos - vec3(_768.x, 0.0, _768.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _797 = (vec2(0.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        float _816 = clamp(log2((length(_41.uCamPos - vec3(_797.x, 0.0, _797.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _826 = (vec2(1.5, 0.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        float _845 = clamp(log2((length(_41.uCamPos - vec3(_826.x, 0.0, _826.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _855 = (vec2(-0.5, 1.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        vec2 _884 = (vec2(0.5, 1.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        float _903 = clamp(log2((length(_41.uCamPos - vec3(_884.x, 0.0, _884.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x);
        vec2 _913 = (vec2(1.5) * _41.uPatchSize + vPatchPosBase[0]) * _41.uScale.xy;
        float _614 = dot(vec4(_787, _816, clamp(log2((length(_41.uCamPos - vec3(_855.x, 0.0, _855.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _903), vec4(0.25));
        float _620 = dot(vec4(clamp(log2((length(_41.uCamPos - vec3(_681.x, 0.0, _681.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _729, _787, _816), vec4(0.25));
        float _626 = dot(vec4(_729, clamp(log2((length(_41.uCamPos - vec3(_739.x, 0.0, _739.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x), _816, _845), vec4(0.25));
        float _632 = dot(vec4(_816, _845, _903, clamp(log2((length(_41.uCamPos - vec3(_913.x, 0.0, _913.y)) + 9.9999997473787516355514526367188e-05) * _41.uDistanceMod), 0.0, _41.uMaxTessLevel.x)), vec4(0.25));
        vec4 _633 = vec4(_614, _620, _626, _632);
        vPatchLods = _633;
        vec4 _940 = exp2(-min(_633, _633.yzwx)) * _41.uMaxTessLevel.y;
        gl_TessLevelOuter[0] = _940.x;
        gl_TessLevelOuter[1] = _940.y;
        gl_TessLevelOuter[2] = _940.z;
        gl_TessLevelOuter[3] = _940.w;
        float _948 = _41.uMaxTessLevel.y * exp2(-min(min(min(_614, _620), min(_626, _632)), _816));
        gl_TessLevelInner[0] = _948;
        gl_TessLevelInner[1] = _948;
    }
}

