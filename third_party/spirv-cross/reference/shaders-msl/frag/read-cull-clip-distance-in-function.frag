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

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
    float gl_CullDistance_0 [[user(cull0)]];
    float gl_CullDistance_1 [[user(cull1)]];
};

static inline __attribute__((always_inline))
float4 read_in_func(thread spvUnsafeArray<float, 2>& gl_CullDistance, thread spvUnsafeArray<float, 2>& gl_ClipDistance)
{
    return float4(gl_CullDistance[0], gl_CullDistance[1], gl_ClipDistance[0], gl_ClipDistance[1]);
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> gl_CullDistance = {};
    spvUnsafeArray<float, 2> gl_ClipDistance = {};
    gl_CullDistance[0] = in.gl_CullDistance_0;
    gl_CullDistance[1] = in.gl_CullDistance_1;
    gl_ClipDistance[0] = in.gl_ClipDistance_0;
    gl_ClipDistance[1] = in.gl_ClipDistance_1;
    out.FragColor = read_in_func(gl_CullDistance, gl_ClipDistance);
    return out;
}

