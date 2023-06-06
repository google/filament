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
    float b = 10.0;
    b = 20.0;
    out.FragColor = b + b;
    return out;
}

