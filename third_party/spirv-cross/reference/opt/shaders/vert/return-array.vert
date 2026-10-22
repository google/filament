#version 310 es

layout(location = 1) in vec4 vInput1;

void main()
{
    gl_Position = vec4(10.0) + vInput1;
}

