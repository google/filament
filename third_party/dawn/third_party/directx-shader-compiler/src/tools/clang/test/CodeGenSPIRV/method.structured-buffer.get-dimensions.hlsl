// RUN: %dxc -T ps_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

struct SBuffer {
  float4   f1;
  float2x3 f2[3];
};

  StructuredBuffer<SBuffer> mySBuffer1;
RWStructuredBuffer<SBuffer> mySBuffer2;

void main() {
  uint numStructs, stride;

// CHECK:      [[len1:%[0-9]+]] = OpArrayLength %uint %mySBuffer1 0
// CHECK-NEXT:                 OpStore %numStructs [[len1]]
// CHECK-NEXT:                 OpStore %stride %uint_96
  mySBuffer1.GetDimensions(numStructs, stride);

// CHECK:      [[len2:%[0-9]+]] = OpArrayLength %uint %mySBuffer2 0
// CHECK-NEXT:                 OpStore %numStructs [[len2]]
// CHECK-NEXT:                 OpStore %stride %uint_96
  mySBuffer2.GetDimensions(numStructs, stride);
}
