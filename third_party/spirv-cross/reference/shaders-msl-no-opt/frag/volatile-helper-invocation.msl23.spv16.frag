#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    bool _12 = simd_is_helper_thread();
    float _15 = float(_12);
    out.FragColor = _15;
    discard_fragment();
    bool _16 = simd_is_helper_thread();
    float _17 = float(_16);
    out.FragColor = _17;
    return out;
}

