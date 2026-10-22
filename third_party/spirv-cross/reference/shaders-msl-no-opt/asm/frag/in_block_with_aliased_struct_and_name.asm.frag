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
    float ALIAS_0_a [[user(locn1)]];
    float ALIAS_0_b [[user(locn2)]];
    float ALIAS_1_a [[user(locn3)]];
    float ALIAS_1_b [[user(locn4)]];
    float ALIAS_2_a [[user(locn5)]];
    float ALIAS_2_b [[user(locn6)]];
    float ALIAS_3_a [[user(locn7)]];
    float ALIAS_3_b [[user(locn8)]];
    float ALIAS_1_0_a [[user(locn10)]];
    float ALIAS_1_0_b [[user(locn11)]];
    float ALIAS_1_1_a [[user(locn12)]];
    float ALIAS_1_1_b [[user(locn13)]];
    float ALIAS_1_2_a [[user(locn14)]];
    float ALIAS_1_2_b [[user(locn15)]];
    float ALIAS_1_3_a [[user(locn16)]];
    float ALIAS_1_3_b [[user(locn17)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<Foo, 4> ALIAS = {};
    spvUnsafeArray<Foo, 4> ALIAS_1 = {};
    ALIAS[0].a = in.ALIAS_0_a;
    ALIAS[0].b = in.ALIAS_0_b;
    ALIAS[1].a = in.ALIAS_1_a;
    ALIAS[1].b = in.ALIAS_1_b;
    ALIAS[2].a = in.ALIAS_2_a;
    ALIAS[2].b = in.ALIAS_2_b;
    ALIAS[3].a = in.ALIAS_3_a;
    ALIAS[3].b = in.ALIAS_3_b;
    ALIAS_1[0].a = in.ALIAS_1_0_a;
    ALIAS_1[0].b = in.ALIAS_1_0_b;
    ALIAS_1[1].a = in.ALIAS_1_1_a;
    ALIAS_1[1].b = in.ALIAS_1_1_b;
    ALIAS_1[2].a = in.ALIAS_1_2_a;
    ALIAS_1[2].b = in.ALIAS_1_2_b;
    ALIAS_1[3].a = in.ALIAS_1_3_a;
    ALIAS_1[3].b = in.ALIAS_1_3_b;
    out.FragColor.x = ALIAS[0].a;
    out.FragColor.y = ALIAS[1].b;
    out.FragColor.z = ALIAS[2].a;
    out.FragColor.w = ALIAS_1[3].b;
    return out;
}

