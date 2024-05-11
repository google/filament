#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;

vec4 foo(vec4 foo_1)
{
    return foo_1 + vec4(1.0);
}

vec4 foo(vec3 foo_1)
{
    return foo_1.xyzz + vec4(1.0);
}

vec4 foo_1(vec4 foo_2)
{
    return foo_2 + vec4(2.0);
}

vec4 foo(vec2 foo_2)
{
    return foo_2.xyxy + vec4(2.0);
}

void main()
{
    highp vec4 foo_3 = vec4(1.0);
    vec4 foo_2 = foo(foo_3);
    highp vec3 foo_5 = vec3(1.0);
    vec4 foo_4 = foo(foo_5);
    highp vec4 foo_7 = vec4(1.0);
    vec4 foo_6 = foo_1(foo_7);
    highp vec2 foo_9 = vec2(1.0);
    vec4 foo_8 = foo(foo_9);
    FragColor = ((foo_2 + foo_4) + foo_6) + foo_8;
}

