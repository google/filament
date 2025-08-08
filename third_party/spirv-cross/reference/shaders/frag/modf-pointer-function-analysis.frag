#version 450

struct ResType
{
    vec4 _m0;
    vec4 _m1;
};

struct ResType_1
{
    float _m0;
    float _m1;
};

layout(location = 0) in vec4 v;
layout(location = 0) out vec4 vo0;
layout(location = 1) out vec4 vo1;

vec4 modf_inner(out vec4 tmp)
{
    ResType _21;
    _21._m0 = modf(v, _21._m1);
    tmp = _21._m1;
    return _21._m0;
}

float modf_inner_partial(inout vec4 tmp)
{
    ResType_1 _34;
    _34._m0 = modf(v.x, _34._m1);
    tmp.x = _34._m1;
    return _34._m0;
}

void main()
{
    vec4 param;
    vec4 _43 = modf_inner(param);
    vec4 tmp = param;
    vo0 = _43;
    vo1 = tmp;
    vec4 param_1 = tmp;
    float _49 = modf_inner_partial(param_1);
    tmp = param_1;
    vo0.x += _49;
    vo1.x += tmp.x;
}

