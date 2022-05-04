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
    SPIRV_Cross_beginInvocationInterlock();
    callee();
    SPIRV_Cross_endInvocationInterlock();
}

