#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Buffer
{
    float4x4 MVPRowMajor;
    float4x4 MVPColMajor;
    float4x4 M;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant Buffer& _13 [[buffer(0)]])
{
    main0_out out = {};
    float4 c0 = _13.M * (in.Position * _13.MVPRowMajor);
    float4 c1 = _13.M * (_13.MVPColMajor * in.Position);
    float4 c2 = _13.M * (_13.MVPRowMajor * in.Position);
    float4 c3 = _13.M * (in.Position * _13.MVPColMajor);
    out.gl_Position = ((c0 + c1) + c2) + c3;
    return out;
}

