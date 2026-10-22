#version 440

layout(std140) uniform AccessedUniformBlock
{
    vec4 a;
    vec4 b;
};

layout(std140) uniform NotAccessedUniformBlock
{
    vec4 c;
    vec4 d;
};

layout(std430) buffer AccessedStorageBlock
{
    float e[512];
};

layout(std430) buffer NotAccessedStorageBlock
{
    float f[512];
};

uniform sampler2D uSampler0; // accessed
uniform sampler2D uSampler1; // not accessed

layout(location = 0) out vec4 a0; // accessed
layout(location = 1) out vec4 a1; // not accessed
layout(location = 2) out vec4 a2; // accessed
layout(location = 3) out vec4 a3; // not accessed

void main()
{
    a0 = a + vec4(e[0]) + texture(uSampler0, vec2(0.5, 0.5));
    a1 = c + vec4(f[1]) + texture(uSampler1, vec2(0.5, 0.5));
    a2 = b + vec4(e[0]) + texture(uSampler0, vec2(0.5, 0.5));
    a3 = d + vec4(f[1]) + texture(uSampler1, vec2(0.5, 0.5));

    gl_Position = vec4(1.0, 1.0, 1.0, 1.0);
}
