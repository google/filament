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
    uint _91 = (!gl_HelperInvocation ? atomic_exchange_explicit((device atomic_uint*)&foo.y, 0u, memory_order_relaxed) : uint{});
    if (int(gl_FragCoord.x) == 3)
    {
        gl_HelperInvocation = true, discard_fragment();
    }
    int2 _101 = int2(gl_FragCoord.xy);
    (gl_HelperInvocation ? ((void)0) : bar.write(uint4(1u), uint2(_101)));
    uint _103 = (!gl_HelperInvocation ? atomic_fetch_add_explicit((device atomic_uint*)&foo.y, 42u, memory_order_relaxed) : uint{});
    uint _108 = (!gl_HelperInvocation ? bar.atomic_fetch_or(uint2(_101), 62u).x : uint{});
    uint _110 = (!gl_HelperInvocation ? atomic_fetch_and_explicit((device atomic_uint*)&foo.y, 65535u, memory_order_relaxed) : uint{});
    uint _112 = (!gl_HelperInvocation ? atomic_fetch_xor_explicit((device atomic_uint*)&foo.y, 4294967040u, memory_order_relaxed) : uint{});
    uint _114 = (!gl_HelperInvocation ? atomic_fetch_min_explicit((device atomic_uint*)&foo.y, 1u, memory_order_relaxed) : uint{});
    uint _119 = (!gl_HelperInvocation ? bar.atomic_fetch_max(uint2(_101), 100u).x : uint{});
    uint _124;
    uint4 _135;
    if (!gl_HelperInvocation)
    {
        do
        {
            _135.x = 100u;
        } while (!bar.atomic_compare_exchange_weak(uint2(_101), &_135, 42u) && _135.x == 100u);
        _124 = _135.x;
    }
    else
    {
        _124 = {};
    }
    bool _125 = gl_HelperInvocation;
    out.fragColor = float4(1.0, float(_125), 0.0, 1.0);
    return out;
}

