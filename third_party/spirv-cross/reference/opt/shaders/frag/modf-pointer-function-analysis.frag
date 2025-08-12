#version 450

layout(location = 0) in vec4 v;
layout(location = 0) out vec4 vo0;
layout(location = 1) out vec4 vo1;

void main()
{
    vec4 param;
    vec4 _59 = modf(v, param);
    vo0 = _59;
    vo1 = param;
    vec4 param_1 = param;
    float _65 = modf(v.x, param_1.x);
    vo0.x += _65;
    vo1.x += param_1.x;
}

