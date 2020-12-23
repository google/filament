#version 310 es
precision mediump float;
precision highp int;

struct Str
{
    mat4 foo;
};

layout(binding = 0, std140) uniform UBO1
{
    layout(row_major) Str foo;
} ubo1;

layout(binding = 1, std140) uniform UBO2
{
    Str foo;
} ubo0;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = ubo1.foo.foo[0] + ubo0.foo.foo[0];
}

