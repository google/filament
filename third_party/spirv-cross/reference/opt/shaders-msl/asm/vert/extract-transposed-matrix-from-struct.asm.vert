#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct InstanceData
{
    float4x4 MATRIX_MVP;
    float4 Color;
};

struct gInstanceData
{
    InstanceData _data[1];
};

struct main0_out
{
    float4 _entryPointOutput_Color [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 PosL [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], const device gInstanceData& gInstanceData_1 [[buffer(0)]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    out.gl_Position = float4(in.PosL, 1.0) * gInstanceData_1._data[gl_InstanceIndex].MATRIX_MVP;
    out._entryPointOutput_Color = gInstanceData_1._data[gl_InstanceIndex].Color;
    return out;
}

