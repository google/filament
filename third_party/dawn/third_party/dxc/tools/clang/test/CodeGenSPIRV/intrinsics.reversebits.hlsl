// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'reversebits' function can only operate on scalar or vector of uints.

void main() {
  uint a;
  uint4 b;
  
// CHECK:      [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT:   {{%[0-9]+}} = OpBitReverse %uint [[a]]
  uint  cb  = reversebits(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4uint %b
// CHECK-NEXT:   {{%[0-9]+}} = OpBitReverse %v4uint [[b]]
  uint4 cb4 = reversebits(b);
}
