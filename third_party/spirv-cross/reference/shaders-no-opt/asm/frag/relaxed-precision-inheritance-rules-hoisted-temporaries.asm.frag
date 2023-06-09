#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec4 FragColor;

void main()
{
    float a = vColor.x;
    highp float b = vColor.y;
    int i = 0;
    float _14;
    highp float hp_copy_14;
    float _15;
    highp float hp_copy_15;
    for (; i < 4; i++, _14 = a, hp_copy_14 = _14, _15 = a * _14, hp_copy_15 = _15, b += (hp_copy_15 * hp_copy_14))
    {
        FragColor += vec4(1.0);
    }
    FragColor += vec4(b);
}

