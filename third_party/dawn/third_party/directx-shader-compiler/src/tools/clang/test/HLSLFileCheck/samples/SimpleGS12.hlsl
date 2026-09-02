// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// Make sure the rowIdx is not immediate.
// CHECK: @dx.op.loadInput.f32(i32 4, i32 0, i32 %

struct VSOut {
  float4 uv[2] : TEXCOORD0;
  float4 pos : SV_Position;
};

struct GSOut {
  float4 pos : SV_Position;
};

uint i;
uint j;

// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)]
[instance(24)]
void main(triangle VSOut points[3], triangle VSOut points2[3] : Overwrite2, inout PointStream<GSOut> stream) {
  GSOut o;
  o.pos = points[j].uv[i] + points2[j].uv[i];
  stream.Append(o);
  stream.RestartStrip();
}