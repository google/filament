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
    mediump ivec4 coords = texelFetch(Buf, ivec2(gl_FragCoord.xy), 0);
    vec4 foo = _34.results[coords.x % 16];
    mediump int c = vIn * vIn;
    mediump int d = vIn2 * vIn2;
    FragColor = (foo + foo) + _34.results[c + d];
}

