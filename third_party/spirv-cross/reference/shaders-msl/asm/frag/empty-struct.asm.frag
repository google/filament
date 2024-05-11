#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct EmptyStructTest
{
};
static inline __attribute__((always_inline))
float GetValue(thread const EmptyStructTest& self)
{
    return 0.0;
}

static inline __attribute__((always_inline))
float GetValue_1(EmptyStructTest self)
{
    return 0.0;
}

fragment void main0()
{
    EmptyStructTest emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(EmptyStructTest{  });
}

