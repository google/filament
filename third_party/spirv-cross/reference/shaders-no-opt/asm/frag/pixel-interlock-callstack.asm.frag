#version 450
#extension GL_ARB_fragment_shader_interlock : require
layout(pixel_interlock_ordered) in;

layout(binding = 1, std430) buffer SSBO1
{
    uint values1[];
} _7;

layout(binding = 0, std430) buffer SSBO0
{
    uint values0[];
} _9;

void callee2()
{
    int _31 = int(gl_FragCoord.x);
    _7.values1[_31]++;
}

void callee()
{
    int _39 = int(gl_FragCoord.x);
    _9.values0[_39]++;
    callee2();
}

void spvMainInterlockedBody()
{
    callee();
}

void main()
{
    // Interlocks were used in a way not compatible with GLSL, this is very slow.
    beginInvocationInterlockARB();
    spvMainInterlockedBody();
    endInvocationInterlockARB();
}
