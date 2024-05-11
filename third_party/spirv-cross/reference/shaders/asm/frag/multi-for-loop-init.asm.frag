#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int counter;

void main()
{
    FragColor = vec4(0.0);
    mediump int i = 0;
    mediump uint j = 1u;
    for (; (i < 10) && (int(j) < int(20u)); i += counter, j += uint(counter))
    {
        FragColor += vec4(float(i));
        FragColor += vec4(float(j));
    }
}

