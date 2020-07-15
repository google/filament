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
    vec2 _204 = vOutPatchPosBase + (gl_TessCoord.xy * _31.uPatchSize);
    vec2 _219 = mix(vPatchLods.yx, vPatchLods.zw, vec2(gl_TessCoord.x));
    float _226 = mix(_219.x, _219.y, gl_TessCoord.y);
    mediump float _228 = floor(_226);
    vec2 _127 = _204 * _31.uInvHeightmapSize;
    vec2 _143 = _31.uInvHeightmapSize * exp2(_228);
    vGradNormalTex = vec4(_127 + (_31.uInvHeightmapSize * 0.5), _127 * _31.uScale.zw);
    mediump vec3 _260 = mix(textureLod(uHeightmapDisplacement, _127 + (_143 * 0.5), _228).xyz, textureLod(uHeightmapDisplacement, _127 + (_143 * 1.0), _228 + 1.0).xyz, vec3(_226 - _228));
    vec2 _173 = (_204 * _31.uScale.xy) + _260.yz;
    vWorld = vec3(_173.x, _260.x, _173.y);
    gl_Position = _31.uMVP * vec4(vWorld, 1.0);
}

