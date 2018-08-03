#version 450

struct MyStruct
{
    vec4 color;
};

layout(std140) uniform MyStruct_CB
{
    MyStruct g_MyStruct[4];
} _6;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    vec3 _28;
    _28 = vec3(0.0);
    for (int _31 = 0; _31 < 4; )
    {
        _28 += _6.g_MyStruct[_31].color.xyz;
        _31++;
        continue;
    }
    _entryPointOutput = vec4(_28, 1.0);
}

