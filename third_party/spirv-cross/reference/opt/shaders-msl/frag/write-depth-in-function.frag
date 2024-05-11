#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
    float gl_FragDepth [[depth(any)]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = 1.0;
    out.gl_FragDepth = 0.20000000298023223876953125;
    return out;
}

