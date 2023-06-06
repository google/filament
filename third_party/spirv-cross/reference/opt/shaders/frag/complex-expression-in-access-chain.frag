#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0, std430) buffer UBO
{
    vec4 results[1024];
} _34;

layout(binding = 1) uniform highp isampler2D Buf;

layout(location = 0) flat in mediump int vIn;
layout(location = 1) flat in mediump int vIn2;
layout(location = 0) out vec4 FragColor;

void main()
{
    mediump int _40 = texelFetch(Buf, ivec2(gl_FragCoord.xy), 0).x % 16;
    FragColor = (_34.results[_40] + _34.results[_40]) + _34.results[(vIn * vIn) + (vIn2 * vIn2)];
}

