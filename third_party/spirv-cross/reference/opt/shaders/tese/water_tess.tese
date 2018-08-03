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

void main()
{
    vec2 _201 = vOutPatchPosBase + (gl_TessCoord.xy * _31.uPatchSize);
    vec2 _214 = mix(vPatchLods.yx, vPatchLods.zw, vec2(gl_TessCoord.x));
    float _221 = mix(_214.x, _214.y, gl_TessCoord.y);
    mediump float _223 = floor(_221);
    vec2 _125 = _201 * _31.uInvHeightmapSize;
    vec2 _141 = _31.uInvHeightmapSize * exp2(_223);
    vGradNormalTex = vec4(_125 + (_31.uInvHeightmapSize * 0.5), _125 * _31.uScale.zw);
    mediump vec3 _253 = mix(textureLod(uHeightmapDisplacement, _125 + (_141 * 0.5), _223).xyz, textureLod(uHeightmapDisplacement, _125 + (_141 * 1.0), _223 + 1.0).xyz, vec3(_221 - _223));
    vec2 _171 = (_201 * _31.uScale.xy) + _253.yz;
    vWorld = vec3(_171.x, _253.x, _171.y);
    gl_Position = _31.uMVP * vec4(vWorld, 1.0);
}

