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

constant spvUnsafeArray<float, 3> _17 = spvUnsafeArray<float, 3>({ 1.0, 2.0, 3.0 });
constant spvUnsafeArray<float, 3> _21 = spvUnsafeArray<float, 3>({ 4.0, 5.0, 6.0 });
constant spvUnsafeArray<spvUnsafeArray<float, 3>, 2> _22 = spvUnsafeArray<spvUnsafeArray<float, 3>, 2>({ spvUnsafeArray<float, 3>({ 1.0, 2.0, 3.0 }), spvUnsafeArray<float, 3>({ 4.0, 5.0, 6.0 }) });

struct main0_out
{
    float vOutput [[color(0)]];
};

struct main0_in
{
    int vIndex1 [[user(locn0)]];
    int vIndex2 [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.vOutput = _22[in.vIndex1][in.vIndex2];
    return out;
}

