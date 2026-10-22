// RUN: %dxc -E main -T ps_6_4 -enable-16bit-types %s | FileCheck %s

// CHECK:       ; Note: shader requires additional functionality:
// CHECK-NEXT:  ;       Use native low precision

// CHECK: call float @dx.op.dot2AddHalf.f32(i32 162,

float2 main(half4 inputs : Inputs0, int acc : Acc0) : SV_Target {
  acc = dot2add(inputs.xy, inputs.zw, acc);
  return acc;
}
