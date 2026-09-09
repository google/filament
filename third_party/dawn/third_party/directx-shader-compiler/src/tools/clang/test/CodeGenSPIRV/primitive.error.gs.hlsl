// RUN: not %dxc -T gs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct GS_OUT
{
  float4 position : SV_POSITION;
  float4 color    : COLOR0;
  float2 uv       : TEXCOORD0;
};

[maxvertexcount(3)]
void main(triangle in    uint i[3] : TriangleVertexID,
          line     in    uint j[2] : LineVertexID,
                   inout PointStream<GS_OUT> out1,
                   inout LineStream<GS_OUT>  out2) {}

// CHECK: error: only one input primitive type can be specified in the geometry shader
// CHECK: error: only one output primitive type can be specified in the geometry shader
