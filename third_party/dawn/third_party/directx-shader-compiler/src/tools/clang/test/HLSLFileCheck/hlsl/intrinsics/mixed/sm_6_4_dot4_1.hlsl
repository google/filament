// RUN: %dxc -E main -T ps_6_4 %s | FileCheck %s

// CHECK: call i32 @dx.op.dot4AddPacked.i32(i32 164,
// CHECK: call i32 @dx.op.dot4AddPacked.i32(i32 163,

float2 f2;

float2 main(uint4 inputs : Inputs0, uint acc0 : Acc0, int acc1 : Acc1) : SV_Target {
  uint acc = 0;
  acc += dot4add_u8packed(inputs.x, inputs.y, acc0) * 2;
  acc += dot4add_i8packed(inputs.z, inputs.w, acc1);
  return f2 * acc;
}
