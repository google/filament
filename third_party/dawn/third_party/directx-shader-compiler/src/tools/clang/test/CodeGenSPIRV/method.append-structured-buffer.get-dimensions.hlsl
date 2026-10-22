// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

struct S {
    float a;
    float3 b;
    float2x3 c;
};

AppendStructuredBuffer<S> buffer;

void main() {
  uint numStructs, stride;

// CHECK:      [[len:%[0-9]+]] = OpArrayLength %uint %buffer 0
// CHECK-NEXT: OpStore %numStructs [[len]]
// CHECK-NEXT: OpStore %stride %uint_64
  buffer.GetDimensions(numStructs, stride);
}
