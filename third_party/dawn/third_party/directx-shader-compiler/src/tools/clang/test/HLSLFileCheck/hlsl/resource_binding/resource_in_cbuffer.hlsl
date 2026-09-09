// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// Make sure structures with only resources have resource fields removed,
// are considered empty, take no space in CBuffer, and do not force alignment.
// CHECK: cbuffer $Globals
// CHECK:   struct hostlayout.$Globals
// CHECK:       float h;                                      ; Offset:    0
// CHECK:       struct hostlayout.struct.LegacyTex
// CHECK:           /* empty struct */
// CHECK:       } tx1;                                        ; Offset:    4
// CHECK:       float i;                                      ; Offset:    4
// CHECK:   } $Globals;                                       ; Offset:    0 Size:     8
// CHECK: cbuffer CB0
// CHECK:   struct hostlayout.CB0
// CHECK:       float f;                                      ; Offset:    0
// CHECK:       struct hostlayout.struct.LegacyTex
// CHECK:           /* empty struct */
// CHECK:       } tx0;                                        ; Offset:    4
// CHECK:       float g;                                      ; Offset:    4
// CHECK:   } CB0;                                            ; Offset:    0 Size:     8

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
  SamplerState  s;
};

cbuffer CB0 {
  float f;          // CB0[0].x
  LegacyTex tx0;    // t0, s0
  float g;          // CB0[0].y
}
Buffer<float4> b0;  // t1
float h;            // $Globals[0].x
LegacyTex tx1;      // t2, s1
float i;            // $Globals[0].y
Buffer<float4> b1;  // t3

float4 tex2D(LegacyTex tx, float2 uv)
{
  return tx.t.Sample(tx.s,uv);
}

float4 main(float2 uv:UV) : SV_Target
{
  return h * b0[(uint)uv.y] * tex2D(tx1,uv) + b1[(uint)uv.x] * tex2D(tx0,uv) * f + g + i;
}
