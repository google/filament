#version 100
#extension GL_EXT_shadow_samplers : require
precision mediump float;
precision highp int;

uniform highp sampler2D uSamp;
uniform highp sampler2DShadow uSampShadow;

varying highp vec4 vUV;

void main()
{
    gl_FragData[0] = texture2D(uSamp, vec2(vUV.x, 0.0));
    gl_FragData[0] += texture2DProj(uSamp, vec3(vUV.xy.x, 0.0, vUV.xy.y));
    gl_FragData[0] += vec4(shadow2DEXT(uSampShadow, vec3(vUV.xyz.x, 0.0, vUV.xyz.z)));
    highp vec4 _44 = vUV;
    highp vec4 _47 = _44;
    _47.y = _44.w;
    gl_FragData[0] += vec4(shadow2DProjEXT(uSampShadow, vec4(_47.x, 0.0, _44.z, _47.y)));
}

