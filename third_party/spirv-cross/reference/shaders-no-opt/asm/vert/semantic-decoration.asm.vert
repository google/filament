#version 450

struct VOut
{
    vec4 p;
    vec4 c;
};

layout(location = 0) out vec4 _entryPointOutput_c;

VOut _main()
{
    VOut v;
    v.p = vec4(1.0);
    v.c = vec4(2.0);
    return v;
}

void main()
{
    VOut flattenTemp = _main();
    gl_Position = flattenTemp.p;
    _entryPointOutput_c = flattenTemp.c;
}

