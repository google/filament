#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

struct foo_t
{
    float x;
    uint y;
};

struct main0_out
{
    float4 fragColor [[color(0)]];
};

fragment main0_out main0(device foo_t& foo [[buffer(0)]], texture2d<uint, access::read_write> bar [[texture(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    if (!gl_HelperInvocation)
    {
        foo.x = 1.0;
    }
    uint _90 = (!gl_HelperInvocation ? atomic_exchange_explicit((device atomic_uint*)&foo.y, 0u, memory_order_relaxed) : uint{});
    if (int(gl_FragCoord.x) == 3)
    {
        gl_HelperInvocation = true, discard_fragment();
    }
    int2 _100 = int2(gl_FragCoord.xy);
    (gl_HelperInvocation ? ((void)0) : bar.write(uint4(1u), uint2(_100)));
    uint _102 = (!gl_HelperInvocation ? atomic_fetch_add_explicit((device atomic_uint*)&foo.y, 42u, memory_order_relaxed) : uint{});
    uint _107 = (!gl_HelperInvocation ? bar.atomic_fetch_or(uint2(_100), 62u).x : uint{});
    uint _109 = (!gl_HelperInvocation ? atomic_fetch_and_explicit((device atomic_uint*)&foo.y, 65535u, memory_order_relaxed) : uint{});
    uint _111 = (!gl_HelperInvocation ? atomic_fetch_xor_explicit((device atomic_uint*)&foo.y, 4294967040u, memory_order_relaxed) : uint{});
    uint _113 = (!gl_HelperInvocation ? atomic_fetch_min_explicit((device atomic_uint*)&foo.y, 1u, memory_order_relaxed) : uint{});
    uint _118 = (!gl_HelperInvocation ? bar.atomic_fetch_max(uint2(_100), 100u).x : uint{});
    uint _123;
    uint4 _131;
    if (!gl_HelperInvocation)
    {
        do
        {
            _131.x = 100u;
        } while (!bar.atomic_compare_exchange_weak(uint2(_100), &_131, 42u) && _131.x == 100u);
        _123 = _131.x;
    }
    else
    {
        _123 = {};
    }
    out.fragColor = float4(1.0, 0.0, 0.0, 1.0);
    return out;
}

