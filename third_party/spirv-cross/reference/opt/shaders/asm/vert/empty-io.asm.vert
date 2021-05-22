#version 450

struct VSOutput
{
    int empty_struct_member;
};

layout(location = 0) in vec4 position;
layout(location = 0) out VSOutput _entryPointOutput;

void main()
{
    gl_Position = position;
}

