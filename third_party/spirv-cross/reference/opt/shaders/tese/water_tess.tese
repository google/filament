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
    vec2 _202 = vOutPatchPosBase + (gl_TessCoord.xy * _31.uPatchSize);
    vec2 _216 = mix(vPatchLods.yx, vPatchLods.zw, vec2(gl_TessCoord.x));
    float _223 = mix(_216.x, _216.y, gl_TessCoord.y);
    mediump float _225 = floor(_223);
    vec2 _125 = _202 * _31.uInvHeightmapSize;
    vec2 _141 = _31.uInvHeightmapSize * exp2(_225);
    vGradNormalTex = vec4(_125 + (_31.uInvHeightmapSize * 0.5), _125 * _31.uScale.zw);
    mediump vec3 _256 = mix(textureLod(uHeightmapDisplacement, _125 + (_141 * 0.5), _225).xyz, textureLod(uHeightmapDisplacement, _125 + (_141 * 1.0), _225 + 1.0).xyz, vec3(_223 - _225));
    vec2 _171 = (_202 * _31.uScale.xy) + _256.yz;
    vWorld = vec3(_171.x, _256.x, _171.y);
    gl_Position = _31.uMVP * vec4(vWorld, 1.0);
}

