// RUN: %dxc -E main -T ps_6_3 -enable-16bit-types %s | FileCheck %s

// CHECK: Opcode Dot2AddHalf not valid in shader model ps_6_3

float2 main(half4 inputs : Inputs0, float acc : Acc0) : SV_Target {
  acc = dot2add(inputs.xy, inputs.zw, acc);
  return acc;
}
