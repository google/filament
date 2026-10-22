// RUN: %dxc -E main -T ps_6_1 %s | FileCheck %s
// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_1 %s | FileCheck %s -check-prefixes=CHECK,CHK16


// CHK16: Note: shader requires additional functionality:
// CHK16-NEXT: Barycentrics

// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 0, i32 0, i8 0, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 0, i32 0, i8 1, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 0, i32 0, i8 2, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 0, i32 0, i8 3, i8 0)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 0, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 1, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 2, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 1, i32 0, i8 3, i8 1)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 2, i32 0, i8 0, i8 2)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 2, i32 0, i8 1, i8 2)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 2, i32 0, i8 2, i8 2)
// CHECK: call float @dx.op.attributeAtVertex.f32(i32 137, i32 2, i32 0, i8 3, i8 2)

float4 main(nointerpolation float4 a : A, nointerpolation float4 b : B, nointerpolation float4 c : C) : SV_Target
{
  float4 a0 = GetAttributeAtVertex(a, 0);
  float4 b1 = GetAttributeAtVertex(b, 1);
  float4 c2 = GetAttributeAtVertex(c, 2);

  return a0 + b1 + c2;
}