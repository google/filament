#version 310 es
precision mediump float;

layout(location = 0) in vec4 vTex;
layout(binding = 0) uniform sampler2D uTex;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = textureProj(uTex, vTex);
}

