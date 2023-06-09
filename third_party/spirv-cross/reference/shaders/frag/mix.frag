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
    bvec4 l = bvec4(false, true, false, false);
    FragColor = mix(vIn0, vIn1, l);
    bool f = true;
    FragColor = vec4(f ? vIn3 : vIn2);
    FragColor = mix(vIn1, vIn0, bvec4(f));
    FragColor = vec4(f ? vIn2 : vIn3);
}

