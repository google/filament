#version 310 es
precision mediump float;

layout(location = 0) in vec4 vIn0;
layout(location = 1) in vec4 vIn1;
layout(location = 2) in float vIn2;
layout(location = 3) in float vIn3;
layout(location = 0) out vec4 FragColor;

void main()
{
    bvec4 l = bvec4(false, true, false, false);
    FragColor = mix(vIn0, vIn1, l);

    bool f = true;
    FragColor = vec4(mix(vIn2, vIn3, f));

    FragColor = f ? vIn0 : vIn1;
    FragColor = vec4(f ? vIn2 : vIn3);
}
