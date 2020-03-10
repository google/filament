#version 450
#extension GL_ARB_fragment_shader_interlock : require
layout(sample_interlock_unordered) in;

layout(binding = 2, std430) coherent buffer Buffer
{
    int foo;
    uint bar;
} _30;

layout(binding = 0, rgba8) uniform writeonly image2D img;
layout(binding = 1, r32ui) uniform uimage2D img2;

void main()
{
    beginInvocationInterlockARB();
    imageStore(img, ivec2(0), vec4(1.0, 0.0, 0.0, 1.0));
    uint _27 = imageAtomicAdd(img2, ivec2(0), 1u);
    _30.foo += 42;
    uint _41 = atomicAnd(_30.bar, 255u);
    endInvocationInterlockARB();
}

