#version 310 es
precision mediump float;
precision highp int;

struct Foo
{
    float var1;
    float var2;
};

layout(binding = 0) uniform mediump sampler2D uSampler;

layout(location = 0) out vec4 FragColor;

Foo _22;

void main()
{
    FragColor = texture(uSampler, vec2(_22.var1, _22.var2));
}

