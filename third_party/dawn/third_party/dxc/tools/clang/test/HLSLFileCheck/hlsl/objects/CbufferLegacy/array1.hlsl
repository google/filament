// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 %s | FileCheck %s


// CHECK: cbuffer $Globals
// CHECK:{
// CHECK:   struct $Globals
// CHECK:{
// CHECK:   struct struct.A
// CHECK:{
// CHECK: float4 x[2];                              ; Offset:    0
// CHECK: uint y[1];                                ; Offset:   32
// CHECK: } a[2];;                                      ; Offset:    0
// CHECK: } $Globals;                                       ; Offset:    0 Size:    84

// CHECK: cbuffer cb
// CHECK: {
// CHECK: struct cb
// CHECK: {
// CHECK: float b;                                      ; Offset:    0
// CHECK: float c[1];                                   ; Offset:   16
// CHECK: } cb;                                             ; Offset:    0 Size:    20

struct A {
  float4 x[2];
  uint   y[1];
};

A a[2];

cbuffer cb {
  float b;
  float c[1];
}

float main() : SV_Target {
   return a[0].y[0] + c[0];
}