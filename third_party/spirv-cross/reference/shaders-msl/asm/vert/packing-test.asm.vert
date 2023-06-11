#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct TestStruct
{
    float4x4 transforms[6];
};

struct CB0
{
    TestStruct CB0[16];
};

vertex void main0()
{
}

