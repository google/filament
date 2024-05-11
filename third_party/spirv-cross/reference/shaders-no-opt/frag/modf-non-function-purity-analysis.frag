#version 450

layout(location = 0) in vec4 v;
layout(location = 1) out vec4 vo1;
layout(location = 0) out vec4 vo0;

vec4 modf_inner()
{
    vec4 _16 = modf(v, vo1);
    return _16;
}

void main()
{
    vec4 _20 = modf_inner();
    vo0 = _20;
}

