#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct storage_block
{
    uint4 baz;
    int2 quux;
};

struct constant_block
{
    float4 foo;
    int bar;
};

#ifndef SPIRV_CROSS_CONSTANT_ID_0
#define SPIRV_CROSS_CONSTANT_ID_0 3
#endif
constant int arraySize = SPIRV_CROSS_CONSTANT_ID_0;

vertex void main0(device storage_block* storage_0 [[buffer(0)]], device storage_block* storage_1 [[buffer(1)]], constant constant_block* constants_0 [[buffer(2)]], constant constant_block* constants_1 [[buffer(3)]], constant constant_block* constants_2 [[buffer(4)]], constant constant_block* constants_3 [[buffer(5)]], array<texture2d<int>, 3> images [[texture(0)]])
{
    device storage_block* storage[] =
    {
        storage_0,
        storage_1,
    };

    constant constant_block* constants[] =
    {
        constants_0,
        constants_1,
        constants_2,
        constants_3,
    };

    storage[0]->baz = uint4(constants[3]->foo);
    storage[1]->quux = images[2].read(uint2(int2(constants[1]->bar))).xy;
}

