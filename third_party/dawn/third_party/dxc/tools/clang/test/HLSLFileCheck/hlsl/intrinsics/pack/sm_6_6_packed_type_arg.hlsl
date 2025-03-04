// RUN: %dxc -T ps_6_6 %s | FileCheck %s

// CHECK: call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
// CHECK: call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 219, i8 1,
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 219, i8 0,
// CHECK: add i32 %{{[0-9]+}}, 5

int foo(uint a) {
  return 1;
}

int foo(int8_t4_packed a) {
  return 2;
}

int foo(uint8_t4_packed a) {
  return 3;
}

int main(int8_t4_packed input1 : Inputs1, uint8_t4_packed input2 : Inputs2) : SV_Target {
  int4 o = unpack_s8s32(input1) + unpack_u8u32(input2);
  return o.x + foo(input1) + foo(input2);
}
