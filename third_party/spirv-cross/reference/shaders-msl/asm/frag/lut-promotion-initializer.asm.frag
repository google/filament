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

constant spvUnsafeArray<float, 16> _46 = spvUnsafeArray<float, 16>({ 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0, 1.0, 2.0, 3.0, 4.0 });
constant spvUnsafeArray<float4, 4> _76 = spvUnsafeArray<float4, 4>({ float4(0.0), float4(1.0), float4(8.0), float4(5.0) });
constant spvUnsafeArray<float4, 4> _90 = spvUnsafeArray<float4, 4>({ float4(20.0), float4(30.0), float4(50.0), float4(60.0) });

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    spvUnsafeArray<float4, 4> foobar = spvUnsafeArray<float4, 4>({ float4(0.0), float4(1.0), float4(8.0), float4(5.0) });
    spvUnsafeArray<float4, 4> baz = spvUnsafeArray<float4, 4>({ float4(0.0), float4(1.0), float4(8.0), float4(5.0) });
    main0_out out = {};
    out.FragColor = _46[in.index];
    if (in.index < 10)
    {
        out.FragColor += _46[in.index ^ 1];
    }
    else
    {
        out.FragColor += _46[in.index & 1];
    }
    if (in.index > 30)
    {
        out.FragColor += _76[in.index & 3].y;
    }
    else
    {
        out.FragColor += _76[in.index & 1].x;
    }
    if (in.index > 30)
    {
        foobar[1].z = 20.0;
    }
    out.FragColor += foobar[in.index & 3].z;
    baz = _90;
    out.FragColor += baz[in.index & 3].z;
    return out;
}

