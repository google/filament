#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vIn0;
layout(location = 1) in vec4 vIn1;
layout(location = 2) in float vIn2;
layout(location = 3) in float vIn3;

void main()
{
    FragColor = vec4(vIn0.x, vIn1.y, vIn0.z, vIn0.w);
    FragColor = vec4(vIn3);
    FragColor = vIn0.xyzw;
    FragColor = vec4(vIn2);
}

