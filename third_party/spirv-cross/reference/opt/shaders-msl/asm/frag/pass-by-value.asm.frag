#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Registers
{
    float foo;
};

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0(constant Registers& registers [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = 10.0 + registers.foo;
    return out;
}

