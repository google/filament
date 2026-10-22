#version 310 es
precision mediump float;
precision highp int;

struct Foo
{
    float var1;
    float var2;
};

Foo _33;

layout(binding = 0) uniform mediump sampler2D uSampler;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(uSampler, vec2(_33.var1, _33.var2));
}

