#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

struct VSOut
{
    float a;
};

layout(location = 0) out VSOut _entryPointOutput;

void main()
{
    _entryPointOutput.a = 40.0;
    gl_Position = vec4(1.0);
}

