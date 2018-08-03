#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Test
{
    int empty_struct_member;
};

vertex void main0()
{
    Test _14 = Test{ 0 };
    Test t = _14;
}

