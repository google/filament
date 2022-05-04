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

layout(binding = 0, std430) buffer SSBO0
{
    uint values0[];
} _9;

void callee2()
{
    int _37 = int(gl_FragCoord.x);
    _7.values1[_37]++;
}

void callee()
{
    int _45 = int(gl_FragCoord.x);
    _9.values0[_45]++;
    callee2();
}

void _29()
{
}

void _31()
{
}

void spvMainInterlockedBody()
{
    callee();
    _29();
    _31();
}

void main()
{
    // Interlocks were used in a way not compatible with GLSL, this is very slow.
    SPIRV_Cross_beginInvocationInterlock();
    spvMainInterlockedBody();
    SPIRV_Cross_endInvocationInterlock();
}
