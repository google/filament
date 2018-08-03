#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uTex;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTex;

vec4 sample_texture(mediump sampler2D tex, vec2 uv)
{
    return texture(tex, uv);
}

void main()
{
    highp vec2 param = vTex;
    FragColor = vColor * sample_texture(uTex, param);
}

