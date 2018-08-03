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
    out.gl_Position = (((_13.M * (in.Position * _13.MVPRowMajor)) + (_13.M * (_13.MVPColMajor * in.Position))) + (_13.M * (_13.MVPRowMajor * in.Position))) + (_13.M * (in.Position * _13.MVPColMajor));
    return out;
}

