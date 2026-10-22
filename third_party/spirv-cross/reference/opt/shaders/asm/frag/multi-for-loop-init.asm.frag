#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int counter;

void main()
{
    FragColor = vec4(0.0);
    mediump int _53 = 0;
    mediump uint _54 = 1u;
    for (; (_53 < 10) && (int(_54) < int(20u)); )
    {
        FragColor += vec4(float(_53));
        FragColor += vec4(float(_54));
        _54 += uint(counter);
        _53 += counter;
        continue;
    }
}

