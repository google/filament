#version 450

layout(location = 0) in float v0;
layout(location = 1) in float v1;
layout(location = 0) out float FragColor;

void main()
{
    float a = v0;
    float b = v1;
    float _16 = a;
    a = v1;
    FragColor = (_16 + b) * b;
}

