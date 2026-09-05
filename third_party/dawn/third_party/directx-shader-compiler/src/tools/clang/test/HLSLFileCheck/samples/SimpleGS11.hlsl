// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: Input signature:
// CHECK: TEXCOORD
// CHECK: COLOR
// CHECK: SV_Position
// CHECK: Overwrite
// CHECK: Overwrite
// CHECK: Overwrite
// CHECK: emitStream
// CHECK: cutStream
// CHECK: i32 24}

struct GSOut {
  float2 uv : TEXCOORD0;
  float4 clr : COLOR;
  float4 pos : SV_Position;
};

cbuffer b : register(b0) {
  float2 invViewportSize;
};

// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)]
[instance(24)]
void main(point GSOut points[1], point GSOut points2[1] : Overwrite2, inout PointStream<GSOut> stream) {

  stream.Append(points[0]);
  stream.Append(points2[0]);

  stream.RestartStrip();
}