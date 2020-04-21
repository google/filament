#version 450
#extension GL_ARB_fragment_shader_interlock : require
layout(pixel_interlock_ordered) in;

layout(binding = 1, std430) buffer SSBO1
{
    uint values1[];
} _14;

layout(binding = 0, std430) buffer SSBO0
{
    uint values0[];
} _35;

void callee2()
{
    int _25 = int(gl_FragCoord.x);
    _14.values1[_25]++;
}

void callee()
{
    int _38 = int(gl_FragCoord.x);
    _35.values0[_38]++;
    callee2();
}

void main()
{
    beginInvocationInterlockARB();
    callee();
    endInvocationInterlockARB();
}

