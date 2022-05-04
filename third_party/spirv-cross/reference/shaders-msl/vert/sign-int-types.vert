#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Implementation of the GLSL sign() function for integer types
template<typename T, typename E = typename enable_if<is_integral<T>::value>::type>
inline T sign(T x)
{
    return select(select(select(x, T(0), x == T(0)), T(1), x > T(0)), T(-1), x < T(0));
}

struct UBO
{
    float4x4 uMVP;
    float4 uFloatVec4;
    float3 uFloatVec3;
    float2 uFloatVec2;
    float uFloat;
    int4 uIntVec4;
    int3 uIntVec3;
    int2 uIntVec2;
    int uInt;
};

struct main0_out
{
    float4 vFloatVec4 [[user(locn0)]];
    float3 vFloatVec3 [[user(locn1)]];
    float2 vFloatVec2 [[user(locn2)]];
    float vFloat [[user(locn3)]];
    int4 vIntVec4 [[user(locn4)]];
    int3 vIntVec3 [[user(locn5)]];
    int2 vIntVec2 [[user(locn6)]];
    int vInt [[user(locn7)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _21 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _21.uMVP * in.aVertex;
    out.vFloatVec4 = sign(_21.uFloatVec4);
    out.vFloatVec3 = sign(_21.uFloatVec3);
    out.vFloatVec2 = sign(_21.uFloatVec2);
    out.vFloat = sign(_21.uFloat);
    out.vIntVec4 = sign(_21.uIntVec4);
    out.vIntVec3 = sign(_21.uIntVec3);
    out.vIntVec2 = sign(_21.uIntVec2);
    out.vInt = sign(_21.uInt);
    return out;
}

