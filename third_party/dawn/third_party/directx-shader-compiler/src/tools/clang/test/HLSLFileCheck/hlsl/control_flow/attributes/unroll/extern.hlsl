// RUN: %dxc -T lib_6_3 %s | FileCheck %s
// RUN: %dxc -T lib_6_6 %s | FileCheck %s

// Global array with external linkage does not need constant indexing.
// Check that the block is not included in the unroll and only happens
// once

// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK-NOT: call i32 @dx.op.bufferUpdateCounter

extern AppendStructuredBuffer<float> buffs[4];

export float f(int arg : A) {
  
  float result = 0;

  [unroll]
  for (int i = 0; i < 4; i++) {
    if (i == arg) {
      buffs[i].Append(arg);
      return 1;
    }
  }
  return 0;
}
