// RUN: %dxc -T lib_6_6 %s -spirv | FileCheck %s

// The inputs for the vertex shader are 0 and 1. vId is a builtin and does not
// get a location.
// CHECK-DAG: OpDecorate %in_var_COLOR0 Location 0
// CHECK-DAG: OpDecorate %in_var_COLOR1 Location 1
struct VIN {
  uint vId : SV_VertexID;
  float2 col0 : COLOR0;
  float2 col1 : COLOR1;
};

// The output for the vertex shader and the input for the pixel shader should
// both be 0. Position is a builtin, and each shader should restart their
// numbering at 0.
// CHECK-DAG: OpDecorate %in_var_COLOR2 Location 0
// CHECK-DAG: OpDecorate %out_var_COLOR2 Location 0
struct V2P
{
    float4 Pos : SV_Position;
    float2 Uv : COLOR2;
};

#define PI (3.14159f)

[shader("vertex")]
V2P VSMain(VIN vIn)
{
    float2 uv = vIn.col0 + vIn.col1;
    V2P vsOut;
    vsOut.Uv = float2(uv.x, 1.0 - uv.y);
    vsOut.Pos = float4((2.0 * uv) - 1.0, 0.0, 1.0);
    return vsOut;
}

[[vk::binding(0)]] Texture2D<float3> source_tex;
[[vk::binding(1)]] SamplerState samp;

// The location for the sole output from the pixel shader should be 0. The
// numbering should restart at 0 for each shader.
// CHECK-DAG: OpDecorate %out_var_SV_Target0 Location 0
[shader("pixel")]
float4 PSMain(V2P psIn) : SV_Target0
{
    return float4(source_tex.Sample(samp, psIn.Uv), 1.0);
}
