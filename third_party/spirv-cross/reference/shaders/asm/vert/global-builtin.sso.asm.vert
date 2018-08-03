#version 450

out gl_PerVertex
{
    vec4 gl_Position;
};

struct VSOut
{
    float a;
    vec4 pos;
};

struct VSOut_1
{
    float a;
};

layout(location = 0) out VSOut_1 _entryPointOutput;

VSOut _main()
{
    VSOut vout;
    vout.a = 40.0;
    vout.pos = vec4(1.0);
    return vout;
}

void main()
{
    VSOut flattenTemp = _main();
    _entryPointOutput.a = flattenTemp.a;
    gl_Position = flattenTemp.pos;
}

