// RUN: %dxc -E main -T gs_6_0 %s | FileCheck %s

// CHECK: SV_RenderTargetArrayIndex or SV_ViewportArrayIndex from any shader feeding rasterizer

// CHECK: InputPrimitive=point
// CHECK: OutputTopology=triangle
// CHECK: OutputStreamMask=1
// CHECK: OutputPositionPresent=1

// CHECK: emitStream
// CHECK: emitStream
// CHECK: emitStream
// CHECK: cutStream

struct VSOut {
  float2 uv : TEXCOORD0;
  float4 clr : COLOR;
  float4 pos : SV_Position;
};

struct VSOutGSIn {
  float3 posSize : POSSIZE;
  float4 clr : COLOR;
  uint index :SV_RenderTargetArrayIndex;
};

struct VSOutGSArrayIn {
  float3 posSize : POSSIZE;
  float2 clr[2] : COLOR;
};

struct VSOutGSMatIn {
  float3 posSize : POSSIZE;
  float2x2 clr[2] : COLOR;
};

cbuffer b : register(b0) {
  float2 invViewportSize;
};

float4 NDC(float2 screen) {
  screen *= invViewportSize * 2;
  screen.x = screen.x - 1;
  screen.y = 1 - screen.y;

  return float4(screen, 0.5f, 1);
}

// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)] void main(point VSOutGSIn points[1], inout TriangleStream<VSOut> stream) {
  VSOut v;

  const float2 verts[3] =
      {
          float2(-0.5f, -0.5f),
          float2(1.5f, -0.5f),
          float2(-0.5f, 1.5f)};

  const float sz = points[0].posSize.z;
  const float2 org = points[0].posSize.xy;
  const float4 clr = float4(points[0].clr); //[0][1], points[ 0 ].clr[1][0]);

  // triangle strip for the particle

  v.uv = float2(0, 0);
  v.clr = clr;
  v.pos = NDC(org + verts[0] * sz);

  stream.Append(v);

  v.uv = float2(2, 0);
  v.clr = clr;
  v.pos = NDC(org + verts[1] * sz);

  stream.Append(v);

  v.uv = float2(0, 2);
  v.clr = clr;
  v.pos = NDC(org + verts[2] * sz);

  stream.Append(v);
  stream.RestartStrip();
}