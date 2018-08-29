#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct EmptyStructTest
{
    int empty_struct_member;
};

float GetValue(thread const EmptyStructTest& self)
{
    return 0.0;
}

float GetValue_1(EmptyStructTest self)
{
    return 0.0;
}

fragment void main0()
{
    EmptyStructTest _23 = EmptyStructTest{ 0 };
    EmptyStructTest emptyStruct;
    float value = GetValue(emptyStruct);
    value = GetValue_1(_23);
}

