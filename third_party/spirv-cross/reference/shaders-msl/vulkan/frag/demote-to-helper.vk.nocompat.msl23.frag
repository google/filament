#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

static inline __attribute__((always_inline))
void foo(thread bool& gl_HelperInvocation)
{
    gl_HelperInvocation = true, discard_fragment();
}

static inline __attribute__((always_inline))
void bar(thread bool& gl_HelperInvocation)
{
    bool _13 = gl_HelperInvocation;
    bool helper = _13;
}

fragment void main0()
{
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    foo(gl_HelperInvocation);
    bar(gl_HelperInvocation);
}

