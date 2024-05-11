#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0(texture2d_ms<float> uImageMS [[texture(0)]], texture2d_array<float, access::read_write> uImageArray [[texture(1)]], texture2d<float, access::write> uImage [[texture(2)]])
{
    uImage.write(uImageMS.read(uint2(int2(1, 2)), 2), uint2(int2(2, 3)));
    uImageArray.write(uImageArray.read(uint2(int3(1, 2, 4).xy), uint(int3(1, 2, 4).z)), uint2(int3(2, 3, 7).xy), uint(int3(2, 3, 7).z));
}

