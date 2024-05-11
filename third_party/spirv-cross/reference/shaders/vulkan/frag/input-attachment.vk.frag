#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0) uniform mediump sampler2D uSubpass0;
layout(binding = 1) uniform mediump sampler2D uSubpass1;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texelFetch(uSubpass0, ivec2(gl_FragCoord.xy), 0) + texelFetch(uSubpass1, ivec2(gl_FragCoord.xy), 0);
}

