#version 310 es
precision mediump float;

layout(location = 0) out vec4 FragColor;

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
    layout(column_major) Str foo;
} ubo0;

void main()
{
    FragColor = ubo1.foo.foo[0] + ubo0.foo.foo[0];
}
