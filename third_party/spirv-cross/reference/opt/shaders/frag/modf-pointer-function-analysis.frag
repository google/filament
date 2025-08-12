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

void main()
{
    ResType _65;
    _65._m0 = modf(v, _65._m1);
    vo0 = _65._m0;
    vo1 = _65._m1;
    ResType_1 _73;
    _73._m0 = modf(v.x, _73._m1);
    vo0.x += _73._m0;
    vo1.x += _73._m1;
}

