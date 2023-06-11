#version 450

layout(location = 0, index = 0) out vec4 FragColor0;
layout(location = 0, index = 1) out vec4 FragColor1;

void main()
{
    FragColor0 = vec4(1.0);
    FragColor1 = vec4(2.0);
}

