#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 PLSOut0 [[color(0)]];
    float4 PLSOut1 [[color(1)]];
    float4 PLSOut2 [[color(2)]];
    float4 PLSOut3 [[color(3)]];
};

struct main0_in
{
    float4 PLSIn0 [[user(locn0)]];
    float4 PLSIn1 [[user(locn1)]];
    float4 PLSIn2 [[user(locn2)]];
    float4 PLSIn3 [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.PLSOut0 = in.PLSIn0 * 2.0;
    out.PLSOut1 = in.PLSIn1 * 6.0;
    out.PLSOut2 = in.PLSIn2 * 7.0;
    out.PLSOut3 = in.PLSIn3 * 4.0;
    return out;
}

