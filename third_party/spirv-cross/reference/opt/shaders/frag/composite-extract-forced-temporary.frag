#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D Texture;

layout(location = 0) in vec2 vTexCoord;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec4 _19 = texture(Texture, vTexCoord);
    float _22 = _19.x;
    FragColor = vec4(_22 * _22);
}

