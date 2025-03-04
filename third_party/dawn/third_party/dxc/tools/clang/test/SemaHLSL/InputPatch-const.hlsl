// RUN: %dxc -Tlib_6_3 -verify %s
// RUN: %dxc -Tgs_6_0 -verify %s

struct GSOut {
  float2 uv : TEXCOORD0;
  float4 clr : COLOR;
  float4 pos : SV_Position;
  float3 norm[2] : NORMAL;
};

cbuffer b : register(b0) {
  float2 invViewportSize;
};

[shader("geometry")]
// geometry shader that outputs 3 vertices from a point
[maxvertexcount(3)]
[instance(24)]
void main(InputPatch<GSOut, 2>points, inout PointStream<GSOut> stream) {

  points[0].norm[0] = 1; // expected-error {{read-only variable is not assignable}} fxc-error {{X3025: l-value specifies const object}}
  points[0].norm[1] = 2; // expected-error {{read-only variable is not assignable}} fxc-error {{X3025: l-value specifies const object}}
  stream.Append(points[0]);

  stream.RestartStrip();
}
