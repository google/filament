// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// Make sure uint isfrontface works.
// CHECK: emitStream
// CHECK: cutStream
// CHECK: i32 24}

struct GSIn {
  float2 uv : TEXCOORD0;
  float4 clr : COLOR;
  float4 pos : SV_Position;

};

struct GSOut {
  float2 uv : TEXCOORD0;
  float4 clr : COLOR;
  float4 pos : SV_Position;
  uint ff : SV_IsFrontFace;
};

cbuffer b : register(b0) {
  float2 invViewportSize;
};

// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)]
[instance(24)]
void main(point GSIn points[1], inout PointStream<GSOut> stream) {

  GSOut o;
  o.uv = points[0].uv;
  o.clr = points[0].clr;
  o.pos = points[0].pos;
  o.ff = false;
  stream.Append(o);

  stream.RestartStrip();
}