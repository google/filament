#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _Block0T
{
    float3x4 World;
};

struct main0_out
{
    float3 _Ret_Val [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant _Block0T& _Block0 [[buffer(0)]])
{
    main0_out out = {};
    float3 _18 = float3(_Block0.World[0][3], _Block0.World[1][3], _Block0.World[2][3]);
    out._Ret_Val = _18;
    return out;
}

