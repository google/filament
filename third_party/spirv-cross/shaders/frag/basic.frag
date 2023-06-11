#version 310 es
precision mediump float;

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTex;
layout(binding = 0) uniform sampler2D uTex;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vColor * texture(uTex, vTex);
}

