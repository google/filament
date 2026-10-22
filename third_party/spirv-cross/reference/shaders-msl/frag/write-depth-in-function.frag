#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
    float gl_FragDepth [[depth(any)]];
};

static inline __attribute__((always_inline))
void set_output_depth(thread float& gl_FragDepth)
{
    gl_FragDepth = 0.20000000298023223876953125;
}

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = 1.0;
    set_output_depth(out.gl_FragDepth);
    return out;
}

