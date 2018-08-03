#version 450

layout(location = 0) in float v0;
layout(location = 1) in float v1;
layout(location = 0) out float FragColor;

void main()
{
    FragColor = (v0 + v1) * v1;
}

