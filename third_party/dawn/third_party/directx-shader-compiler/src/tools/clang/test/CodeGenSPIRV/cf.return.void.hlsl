// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void A() {
}

void main() {
  // CHECK: [[type:%[0-9]+]] = OpTypeFunction %void
  // CHECK:        %src_main = OpFunction %void None [[type]]
  // CHECK:      {{%[0-9]+}} = OpFunctionCall %void %A
  // CHECK:                    OpReturn
  // CHECK:                    OpFunctionEnd
  return A();
}
