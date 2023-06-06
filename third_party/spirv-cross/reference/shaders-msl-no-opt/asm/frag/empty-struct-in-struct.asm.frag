#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct EmptyStructTest
{
};
struct EmptyStruct2Test
{
    EmptyStructTest _m0;
};

static inline __attribute__((always_inline))
float GetValue(thread const EmptyStruct2Test& self)
{
    return 0.0;
}

static inline __attribute__((always_inline))
float GetValue_1(EmptyStruct2Test self)
{
    return 0.0;
}

fragment void main0()
{
    EmptyStruct2Test emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(EmptyStruct2Test{ EmptyStructTest{  } });
    value = GetValue_1(EmptyStruct2Test{ { } });
}

