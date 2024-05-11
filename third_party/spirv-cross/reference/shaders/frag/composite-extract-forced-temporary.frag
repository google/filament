#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D Texture;

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;

void main()
{
    float f = texture(Texture, vTexCoord).x;
    FragColor = vec4(f * f);
}

