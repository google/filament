#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float a[1];
    float2 b[2];
};

struct UBOEnhancedLayout
{
    float c[1];
    float2 d[2];
    char _m2_pad[9976];
    float e;
};

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    int vIndex [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO& _17 [[buffer(0)]], constant UBOEnhancedLayout& _30 [[buffer(1)]])
{
    main0_out out = {};
    out.FragColor = (_17.a[in.vIndex] + _30.c[in.vIndex]) + _30.e;
    return out;
}

