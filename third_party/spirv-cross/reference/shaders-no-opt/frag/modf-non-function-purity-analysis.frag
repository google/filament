#version 450

struct ResType
{
    vec4 _m0;
    vec4 _m1;
};

layout(location = 0) in vec4 v;
layout(location = 1) out vec4 vo1;
layout(location = 0) out vec4 vo0;

vec4 modf_inner()
{
    ResType _17;
    _17._m0 = modf(v, _17._m1);
    vo1 = _17._m1;
    return _17._m0;
}

void main()
{
    vec4 _23 = modf_inner();
    vo0 = _23;
}

