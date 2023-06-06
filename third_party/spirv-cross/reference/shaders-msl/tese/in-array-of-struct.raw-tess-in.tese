#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

struct _35
{
    float dummy;
    float4 variableInStruct;
};

struct main0_out
{
    float outResult [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    spvUnsafeArray<_35, 3> testStructArray;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(float3 gl_TessCoord [[position_in_patch]], uint gl_PrimitiveID [[patch_id]], const device main0_in* spvIn [[buffer(22)]])
{
    main0_out out = {};
    const device main0_in* gl_in = &spvIn[gl_PrimitiveID * 0];
    out.gl_Position = float4((gl_TessCoord.xy * 2.0) - float2(1.0), 0.0, 1.0);
    float result = ((float(abs(gl_in[0].testStructArray[2].variableInStruct.x - (-4.0)) < 0.001000000047497451305389404296875) * float(abs(gl_in[0].testStructArray[2].variableInStruct.y - (-9.0)) < 0.001000000047497451305389404296875)) * float(abs(gl_in[0].testStructArray[2].variableInStruct.z - 3.0) < 0.001000000047497451305389404296875)) * float(abs(gl_in[0].testStructArray[2].variableInStruct.w - 7.0) < 0.001000000047497451305389404296875);
    out.outResult = result;
    return out;
}

