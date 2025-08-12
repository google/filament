#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

struct Output
{
    uint result;
};

struct main0_out
{
    float4 fragColor [[color(0)]];
    float gl_FragDepth [[depth(any)]];
};

fragment main0_out main0(volatile device Output& sb_out [[buffer(0)]])
{
    main0_out out = {};
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    uint _16 = (!gl_HelperInvocation ? atomic_fetch_add_explicit((volatile device atomic_uint*)&sb_out.result, 1u, memory_order_relaxed) : uint{});
    out.gl_FragDepth = 0.75;
    out.fragColor = float4(1.0, 1.0, 0.0, 1.0);
    gl_HelperInvocation = true, discard_fragment();
    return out;
}

