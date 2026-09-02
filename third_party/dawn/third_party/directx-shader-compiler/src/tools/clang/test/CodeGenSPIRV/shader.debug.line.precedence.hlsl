// RUN: %dxc -T cs_6_0 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK:      [[src:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file]]

[numthreads(1,1,1)]void main() {
  int a;
  int b;

//CHECK:      DebugLine [[src]] %uint_12 %uint_12 %uint_11 %uint_11
//CHECK:      OpSelectionMerge %switch_merge None
  switch (a) {
  default:
    b = 0;
  case 1:
    b = 1;
    break;
  case 2:
    b = 2;
  }

//CHECK:      DebugLine [[src]] %uint_24 %uint_24 %uint_19 %uint_23
//CHECK:      OpLoopMerge %for_merge %for_continue None
  for (int i = 0; i < 4; i++) {
    b += i;
  }
}
