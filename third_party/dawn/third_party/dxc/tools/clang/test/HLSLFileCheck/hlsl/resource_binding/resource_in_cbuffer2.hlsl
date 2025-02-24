// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure structures with resources have resource fields removed.
// CHECK: cbuffer $Globals
// CHECK:   struct hostlayout.$Globals
// CHECK:       float h;                                      ; Offset:    0
// CHECK:       struct hostlayout.struct.LegacyTex
// CHECK:           float f;                                  ; Offset:   16
// CHECK:       } tx1;                                        ; Offset:   16
// CHECK:       float i;                                      ; Offset:   20
// CHECK:   } $Globals;                                       ; Offset:    0 Size:    24
// CHECK: cbuffer CB0
// CHECK:   struct hostlayout.CB0
// CHECK:       float f;                                      ; Offset:    0
// CHECK:       struct hostlayout.struct.LegacyTex
// CHECK:           float f;                                  ; Offset:   16
// CHECK:       } tx0;                                        ; Offset:   16
// CHECK:       float g;                                      ; Offset:   20
// CHECK:   } CB0;                                            ; Offset:    0 Size:    24

// CHECK: $Globals                          cbuffer      NA          NA     CB0            cb0     1
// CHECK: CB0                               cbuffer      NA          NA     CB1            cb1     1

// TODO: Preserve delcaration order when allocating resources from structs
// CHECK-DAG: tx0.s                             sampler      NA          NA      S{{.}}             s{{.}}     1
// CHECK-DAG: tx1.s                             sampler      NA          NA      S{{.}}             s{{.}}     1
// CHECK-DAG: b0                                texture     f32         buf      T{{.}}             t{{.}}     1
// CHECK-DAG: b1                                texture     f32         buf      T{{.}}             t{{.}}     1
// CHECK-DAG: tx0.t                             texture     f32          2d      T{{.}}             t{{.}}     1
// CHECK-DAG: tx1.t                             texture     f32          2d      T{{.}}             t{{.}}     1

// bound the check-dags
// CHECK: define void @main() {

struct LegacyTex
{
  Texture2D t;
  float f;
  SamplerState  s;
};

cbuffer CB0 {
  float f;          // CB0[0].x
  LegacyTex tx0;    // t0, s0, CB0[1].x
  float g;          // CB0[2].x
}
Buffer<float4> b0;  // t1
float h;            // $Globals[0].x
LegacyTex tx1;      // t2, s1, $Globals[1].x
float i;            // $Globals[2].x
Buffer<float4> b1;  // t3

float4 tex2D(LegacyTex tx, float2 uv)
{
  return tx.t.Sample(tx.s,uv) * tx.f;
}

float4 main(float2 uv:UV) : SV_Target
{
  return h * b0[(uint)uv.y] * tex2D(tx1,uv) + b1[(uint)uv.x] * tex2D(tx0,uv) * f + g * i;
}
