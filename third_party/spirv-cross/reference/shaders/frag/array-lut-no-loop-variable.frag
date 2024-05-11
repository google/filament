#version 310 es
precision mediump float;
precision highp int;

const float _17[5] = float[](1.0, 2.0, 3.0, 4.0, 5.0);

layout(location = 0) out vec4 FragColor;

void main()
{
    for (mediump int i = 0; i < 4; i++, FragColor += vec4(_17[i]))
    {
    }
}

