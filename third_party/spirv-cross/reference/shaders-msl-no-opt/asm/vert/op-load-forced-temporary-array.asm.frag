#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float _21 = {};

struct main0_out
{
    float4 gl_Position [[position]];
};

// Implementation of an array copy function to cover GLSL's ability to copy an array via assignment.
template<typename T, uint N>
void spvArrayCopyFromStack1(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

template<typename T, uint N>
void spvArrayCopyFromConstant1(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; dst[i] = src[i], i++);
}

vertex main0_out main0()
{
    main0_out out = {};
    float _23[2];
    for (int _25 = 0; _25 < 2; )
    {
        _23[_25] = 0.0;
        _25++;
        continue;
    }
    float _31[2];
    spvArrayCopyFromStack1(_31, _23);
    float _37;
    if (as_type<uint>(3.0) != 0u)
    {
        _37 = _31[0];
    }
    else
    {
        _37 = _21;
    }
    out.gl_Position = float4(0.0, 0.0, 0.0, _37);
    return out;
}

