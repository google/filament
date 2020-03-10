#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct V2F
{
    float4 Position;
    float4 Color;
};

struct InstanceData
{
    float4x4 MATRIX_MVP;
    float4 Color;
};

struct InstanceData_1
{
    float4x4 MATRIX_MVP;
    float4 Color;
};

struct gInstanceData
{
    InstanceData_1 _data[1];
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

static inline __attribute__((always_inline))
V2F _VS(thread const float3& PosL, thread const uint& instanceID, const device gInstanceData& gInstanceData_1)
{
    InstanceData instData;
    instData.MATRIX_MVP = transpose(gInstanceData_1._data[instanceID].MATRIX_MVP);
    instData.Color = gInstanceData_1._data[instanceID].Color;
    V2F v2f;
    v2f.Position = instData.MATRIX_MVP * float4(PosL, 1.0);
    v2f.Color = instData.Color;
    return v2f;
}

vertex main0_out main0(main0_in in [[stage_in]], const device gInstanceData& gInstanceData_1 [[buffer(0)]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    float3 PosL = in.PosL;
    uint instanceID = gl_InstanceIndex;
    float3 param = PosL;
    uint param_1 = instanceID;
    V2F flattenTemp = _VS(param, param_1, gInstanceData_1);
    out.gl_Position = flattenTemp.Position;
    out._entryPointOutput_Color = flattenTemp.Color;
    return out;
}

