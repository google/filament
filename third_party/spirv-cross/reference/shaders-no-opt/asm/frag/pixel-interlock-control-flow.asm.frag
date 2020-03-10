#version 450
#extension GL_ARB_fragment_shader_interlock : require
layout(pixel_interlock_ordered) in;

layout(binding = 1, std430) buffer SSBO1
{
    uint values1[];
} _7;

layout(binding = 2, std430) buffer _12_13
{
    uint _m0[];
} _13;

layout(binding = 0, std430) buffer SSBO0
{
    uint values0[];
} _9;

void callee2()
{
    int _44 = int(gl_FragCoord.x);
    _7.values1[_44]++;
}

void callee()
{
    int _52 = int(gl_FragCoord.x);
    _9.values0[_52]++;
    callee2();
    if (true)
    {
    }
}

void _35()
{
    _13._m0[int(gl_FragCoord.x)] = 4u;
}

void spvMainInterlockedBody()
{
    callee();
    _35();
}

void main()
{
    // Interlocks were used in a way not compatible with GLSL, this is very slow.
    beginInvocationInterlockARB();
    spvMainInterlockedBody();
    endInvocationInterlockARB();
}
