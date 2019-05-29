#version 450
layout(quads) in;

struct HS_INPUT
{
    vec4 foo;
    vec4 bar;
};

struct ControlPoint
{
    vec4 baz;
};

struct DS_OUTPUT
{
    vec4 pos;
};

layout(location = 0) patch in vec4 input_foo;
layout(location = 1) patch in vec4 input_bar;
layout(location = 2) in ControlPoint CPData[];

DS_OUTPUT _main(HS_INPUT _input, vec2 uv, ControlPoint CPData_1[4])
{
    DS_OUTPUT o;
    o.pos = (((_input.foo + _input.bar) + uv.xyxy) + CPData_1[0].baz) + CPData_1[3].baz;
    return o;
}

void main()
{
    HS_INPUT _input;
    _input.foo = input_foo;
    _input.bar = input_bar;
    vec2 uv = vec2(gl_TessCoord.xy);
    ControlPoint _54_unrolled[4];
    for (int i = 0; i < int(4); i++)
    {
        _54_unrolled[i] = CPData[i];
    }
    ControlPoint CPData_1[4] = _54_unrolled;
    HS_INPUT param = _input;
    vec2 param_1 = uv;
    ControlPoint param_2[4] = CPData_1;
    gl_Position = _main(param, param_1, param_2).pos;
}

