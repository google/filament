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

struct Foo
{
    float a;
    float b;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float foos_0_a [[user(locn1)]];
    float foos_0_b [[user(locn2)]];
    float foos_1_a [[user(locn3)]];
    float foos_1_b [[user(locn4)]];
    float foos_2_a [[user(locn5)]];
    float foos_2_b [[user(locn6)]];
    float foos_3_a [[user(locn7)]];
    float foos_3_b [[user(locn8)]];
    float bars_0_a [[user(locn10)]];
    float bars_0_b [[user(locn11)]];
    float bars_1_a [[user(locn12)]];
    float bars_1_b [[user(locn13)]];
    float bars_2_a [[user(locn14)]];
    float bars_2_b [[user(locn15)]];
    float bars_3_a [[user(locn16)]];
    float bars_3_b [[user(locn17)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<Foo, 4> foos = {};
    spvUnsafeArray<Foo, 4> bars = {};
    foos[0].a = in.foos_0_a;
    foos[0].b = in.foos_0_b;
    foos[1].a = in.foos_1_a;
    foos[1].b = in.foos_1_b;
    foos[2].a = in.foos_2_a;
    foos[2].b = in.foos_2_b;
    foos[3].a = in.foos_3_a;
    foos[3].b = in.foos_3_b;
    bars[0].a = in.bars_0_a;
    bars[0].b = in.bars_0_b;
    bars[1].a = in.bars_1_a;
    bars[1].b = in.bars_1_b;
    bars[2].a = in.bars_2_a;
    bars[2].b = in.bars_2_b;
    bars[3].a = in.bars_3_a;
    bars[3].b = in.bars_3_b;
    out.FragColor.x = foos[0].a;
    out.FragColor.y = foos[1].b;
    out.FragColor.z = foos[2].a;
    out.FragColor.w = bars[3].b;
    return out;
}

