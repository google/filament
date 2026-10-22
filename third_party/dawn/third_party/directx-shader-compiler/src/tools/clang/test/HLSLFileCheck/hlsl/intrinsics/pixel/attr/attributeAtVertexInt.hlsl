// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_2 -HV 2018 -enable-16bit-types %s | FileCheck %s


// CHECK: Note: shader requires additional functionality:
// CHECK-NEXT: Barycentrics

// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 0, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 1, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 2, i8 0)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 0, i32 0, i8 3, i8 0)

// CHECK: call i16 @dx.op.attributeAtVertex.i16(i32 137, i32 1, i32 0, i8 0, i8 1)
// CHECK: call i16 @dx.op.attributeAtVertex.i16(i32 137, i32 1, i32 0, i8 1, i8 1)
// CHECK: call i16 @dx.op.attributeAtVertex.i16(i32 137, i32 1, i32 0, i8 2, i8 1)
// CHECK: call i16 @dx.op.attributeAtVertex.i16(i32 137, i32 1, i32 0, i8 3, i8 1)

// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 2, i32 0, i8 0, i8 2)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 2, i32 0, i8 1, i8 2)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 2, i32 0, i8 2, i8 2)
// CHECK: call i32 @dx.op.attributeAtVertex.i32(i32 137, i32 2, i32 0, i8 3, i8 2)

int4 main(nointerpolation int4 a : A, nointerpolation int16_t4 b : B, nointerpolation int4 c : C) : SV_Target
{
  int4 a0 = GetAttributeAtVertex(a, 0);
  int4 b1 = GetAttributeAtVertex(b, 1);
  int4 c2 = GetAttributeAtVertex(c, 2);

  return a0 + b1 + c2;
}