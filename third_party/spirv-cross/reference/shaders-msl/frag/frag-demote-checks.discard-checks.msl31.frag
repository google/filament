#pragma clang diagnostic ignored "-Wmissing-prototypes"
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

static inline __attribute__((always_inline))
float4 frag_body(device foo_t& foo, thread float4& gl_FragCoord, texture2d<uint, access::read_write> bar, thread bool& gl_HelperInvocation)
{
    if (!gl_HelperInvocation)
    {
        foo.x = 1.0;
    }
    uint _25 = (!gl_HelperInvocation ? atomic_exchange_explicit((device atomic_uint*)&foo.y, 0u, memory_order_relaxed) : uint{});
    if (int(gl_FragCoord.x) == 3)
    {
        gl_HelperInvocation = true, discard_fragment();
    }
    (gl_HelperInvocation ? ((void)0) : bar.write(uint4(1u), uint2(int2(gl_FragCoord.xy))));
    uint _50 = (!gl_HelperInvocation ? atomic_fetch_add_explicit((device atomic_uint*)&foo.y, 42u, memory_order_relaxed) : uint{});
    uint _57 = (!gl_HelperInvocation ? bar.atomic_fetch_or(uint2(int2(gl_FragCoord.xy)), 62u).x : uint{});
    uint _60 = (!gl_HelperInvocation ? atomic_fetch_and_explicit((device atomic_uint*)&foo.y, 65535u, memory_order_relaxed) : uint{});
    uint _63 = (!gl_HelperInvocation ? atomic_fetch_xor_explicit((device atomic_uint*)&foo.y, 4294967040u, memory_order_relaxed) : uint{});
    uint _65 = (!gl_HelperInvocation ? atomic_fetch_min_explicit((device atomic_uint*)&foo.y, 1u, memory_order_relaxed) : uint{});
    uint _71 = (!gl_HelperInvocation ? bar.atomic_fetch_max(uint2(int2(gl_FragCoord.xy)), 100u).x : uint{});
    uint _76;
    uint4 _96;
    if (!gl_HelperInvocation)
    {
        do
        {
            _96.x = 100u;
        } while (!bar.atomic_compare_exchange_weak(uint2(int2(gl_FragCoord.xy)), &_96, 42u) && _96.x == 100u);
        _76 = _96.x;
    }
    else
    {
        _76 = {};
    }
    bool _77 = gl_HelperInvocation;
    return float4(1.0, float(_77), 0.0, 1.0);
}

fragment main0_out main0(device foo_t& foo [[buffer(0)]], texture2d<uint, access::read_write> bar [[texture(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    float4 _85 = frag_body(foo, gl_FragCoord, bar, gl_HelperInvocation);
    out.fragColor = _85;
    return out;
}

