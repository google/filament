#version 310 es

layout(binding = 0, std140) uniform Buffer
{
    layout(row_major) mat4 HP;
    layout(row_major) mediump mat4 MP;
} _21;

layout(binding = 1, std140) uniform Buffer2
{
    layout(row_major) mediump mat4 MP2;
} _39;

layout(location = 0) out vec4 H;
layout(location = 0) in vec4 Hin;
layout(location = 1) out mediump vec4 M;
layout(location = 1) in mediump vec4 Min;
layout(location = 2) out mediump vec4 M2;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(1.0);
    H = spvWorkaroundRowMajor(_21.HP) * Hin;
    M = spvWorkaroundRowMajor(_21.MP) * Min;
    M2 = spvWorkaroundRowMajorMP(_39.MP2) * Min;
}

