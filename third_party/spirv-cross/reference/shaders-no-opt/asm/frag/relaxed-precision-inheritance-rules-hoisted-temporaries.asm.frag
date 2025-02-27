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
    float _44;
    highp float hp_copy_44;
    float _45;
    highp float hp_copy_45;
    for (; i < 4; i++, _44 = a, hp_copy_44 = _44, _45 = a * _44, hp_copy_45 = _45, b += (hp_copy_45 * hp_copy_44))
    {
        FragColor += vec4(1.0);
    }
    FragColor += vec4(b);
}

