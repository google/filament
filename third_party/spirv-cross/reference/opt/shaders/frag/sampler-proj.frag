#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uTex;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vTex;

void main()
{
    highp vec4 _19 = vTex;
    _19.z = vTex.w;
    FragColor = textureProj(uTex, _19.xyz);
}

