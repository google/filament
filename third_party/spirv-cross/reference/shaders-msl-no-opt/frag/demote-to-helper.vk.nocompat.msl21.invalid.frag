#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    bool _9 = simd_is_helper_thread();
    bool helper = _9;
}

