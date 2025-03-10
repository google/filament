// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %float_255_001999 = OpConstant %float 255.001999

void main() {
  float4 input;

// CHECK:         [[input:%[0-9]+]] = OpLoad %v4float %input
// CHECK-NEXT: [[swizzled:%[0-9]+]] = OpVectorShuffle %v4float [[input]] [[input]] 2 1 0 3
// CHECK-NEXT:   [[scaled:%[0-9]+]] = OpVectorTimesScalar %v4float [[swizzled]] %float_255_001999
// CHECK-NEXT:          {{%[0-9]+}} = OpConvertFToS %v4int [[scaled]]
  int4 result = D3DCOLORtoUBYTE4(input);
}
