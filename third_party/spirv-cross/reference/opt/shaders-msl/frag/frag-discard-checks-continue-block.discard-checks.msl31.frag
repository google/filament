#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct foo
{
    int x;
};

struct main0_out
{
    float4 fragColor [[color(0)]];
};

fragment main0_out main0(device foo& _24 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    bool gl_HelperInvocation = {};
    gl_HelperInvocation = simd_is_helper_thread();
    if (gl_FragCoord.y == 7.0)
    {
        gl_HelperInvocation = true, discard_fragment();
    }
    if (!gl_HelperInvocation)
    {
        _24.x = 0;
    }
    for (; float(_24.x) < gl_FragCoord.x; )
    {
        if (!gl_HelperInvocation)
        {
            _24.x++;
        }
        continue;
    }
    out.fragColor = float4(float(_24.x), 0.0, 0.0, 1.0);
    return out;
}

