// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: target datalayout = "e-m:e-p:32:32-i1:32-i8:8-i16:16-i32:32-i64:64-f16:16-f32:32-f64:64-n8:16:32:64"

float4 main(float4 a : A) : SV_Target {
  return 1;
}