#version 450
#ifdef GL_ARB_fragment_shader_interlock
#extension GL_ARB_fragment_shader_interlock : enable
#define SPIRV_Cross_beginInvocationInterlock() beginInvocationInterlockARB()
#define SPIRV_Cross_endInvocationInterlock() endInvocationInterlockARB()
#elif defined(GL_INTEL_fragment_shader_ordering)
#extension GL_INTEL_fragment_shader_ordering : enable
#define SPIRV_Cross_beginInvocationInterlock() beginFragmentShaderOrderingINTEL()
#define SPIRV_Cross_endInvocationInterlock()
#endif
#if defined(GL_ARB_fragment_shader_interlock)
layout(pixel_interlock_ordered) in;
#elif !defined(GL_INTEL_fragment_shader_ordering)
#error Fragment Shader Interlock/Ordering extension missing!
#endif

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
    SPIRV_Cross_beginInvocationInterlock();
    spvMainInterlockedBody();
    SPIRV_Cross_endInvocationInterlock();
}
