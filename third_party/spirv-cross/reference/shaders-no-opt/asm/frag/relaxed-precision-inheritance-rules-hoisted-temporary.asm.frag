#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) in float vColor;
layout(location = 0) out float FragColor;

void main()
{
    float b;
    highp float hp_copy_b;
    do
    {
        b = vColor * vColor;
        hp_copy_b = b;
    } while (false);
    FragColor = hp_copy_b * hp_copy_b;
}

