#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2DShadow uT;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec3 vUV;

void main()
{
    FragColor = textureGather(uT, vUV.xy, vUV.z);
}

