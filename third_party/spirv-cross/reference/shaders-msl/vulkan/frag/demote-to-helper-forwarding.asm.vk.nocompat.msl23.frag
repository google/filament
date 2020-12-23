#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    bool _15 = simd_is_helper_thread();
    discard_fragment();
    if (!_15)
    {
        out.FragColor = float4(1.0, 0.0, 0.0, 1.0);
    }
    return out;
}

