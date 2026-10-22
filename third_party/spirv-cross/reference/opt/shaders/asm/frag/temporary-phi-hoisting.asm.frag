#version 450

struct MyStruct
{
    vec4 color;
};

layout(binding = 0, std140) uniform MyStruct_CB
{
    MyStruct g_MyStruct[4];
} _8;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec3 _85;
    _85 = vec3(0.0);
    for (int _86 = 0; _86 < 4; )
    {
        _85 += _8.g_MyStruct[_86].color.xyz;
        _86++;
        continue;
    }
    _entryPointOutput = vec4(_85, 1.0);
}

