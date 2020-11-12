#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    discard_fragment();
    bool _9 = simd_is_helper_thread();
}

