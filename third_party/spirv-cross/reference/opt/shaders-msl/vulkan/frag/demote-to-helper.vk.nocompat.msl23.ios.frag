#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

fragment void main0()
{
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    gl_HelperInvocation = true, discard_fragment();
    bool _9 = gl_HelperInvocation;
}

