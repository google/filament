#version 450

struct VSOutput
{
    int empty_struct_member;
};

layout(location = 0) in vec4 position;

void main()
{
    gl_Position = position;
}

