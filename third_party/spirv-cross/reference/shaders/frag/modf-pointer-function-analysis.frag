#version 450

layout(location = 0) in vec4 v;
layout(location = 0) out vec4 vo0;
layout(location = 1) out vec4 vo1;

vec4 modf_inner(out vec4 tmp)
{
    vec4 _20 = modf(v, tmp);
    return _20;
}

float modf_inner_partial(inout vec4 tmp)
{
    float _30 = modf(v.x, tmp.x);
    return _30;
}

void main()
{
    vec4 param;
    vec4 _37 = modf_inner(param);
    vec4 tmp = param;
    vo0 = _37;
    vo1 = tmp;
    vec4 param_1 = tmp;
    float _43 = modf_inner_partial(param_1);
    tmp = param_1;
    vo0.x += _43;
    vo1.x += tmp.x;
}

