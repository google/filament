// RUN: %dxc -T vs_6_0 -E main -Oconfig=--ssa-rewrite,--if-conversion,--eliminate-dead-code-aggressive  %s -spirv | FileCheck %s

// Note: The above optimization recipe should replace the if-else condition
// with OpSelect, but it should not reduce the whole shader to "return (1,2,1,2)"

// CHECK-NOT: OpConstantComposite %v4float %float_1 %float_2 %float_1 %float_2

float4 main() : SV_POSITION {
  float j = 0;
  float i = 0;
  bool cond = true;

// CHECK: OpSelect %float %true %float_2 %float_20
// CHECK: OpSelect %float %true %float_1 %float_10
  if(cond) {
    i = 1;
    j = 2;
  } else {
    i = 10;
    j = 20;
  }

  return float4(i, j, i, j);
}

