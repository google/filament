// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types  -fcgl  %s -spirv | FileCheck %s

// The 'reversebits' function can only operate on scalar or vector of integers.

// CHECK: [[shift:%[0-9]+]] = OpConstantComposite %v4uint %uint_16 %uint_16 %uint_16 %uint_16
// CHECK: [[v4ulong_32:%[0-9]+]] = OpConstantComposite %v4ulong %ulong_32 %ulong_32 %ulong_32 %ulong_32
void main() {
  uint a;
  uint4 b;
  uint16_t c;
  uint16_t4 d;
  uint64_t e;
  uint64_t4 f;
  
// CHECK:      [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT:   {{%[0-9]+}} = OpBitReverse %uint [[a]]
  uint  ar  = reversebits(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4uint %b
// CHECK-NEXT:   {{%[0-9]+}} = OpBitReverse %v4uint [[b]]
  uint4 br = reversebits(b);

// CHECK:      [[c:%[0-9]+]] = OpLoad %ushort %c
// CHECK-NEXT: [[c32:%[0-9]+]] = OpUConvert %uint [[c]]
// CHECK-NEXT: [[rev:%[0-9]+]] = OpBitReverse %uint [[c32]]
// CHECK-NEXT: [[shr:%[0-9]+]] = OpShiftRightLogical %uint [[rev]] %uint_16
// CHECK-NEXT: {{%[0-9]+}} = OpUConvert %ushort [[shr]]
  uint16_t cr = reversebits(c);


// CHECK:      [[d:%[0-9]+]] = OpLoad %v4ushort %d
// CHECK-NEXT: [[d32:%[0-9]+]] = OpUConvert %v4uint [[d]]
// CHECK-NEXT: [[rev:%[0-9]+]] = OpBitReverse %v4uint [[d32]]
// CHECK-NEXT: [[shr:%[0-9]+]] = OpShiftRightLogical %v4uint [[rev]] [[shift]]
// CHECK-NEXT: {{%[0-9]+}} = OpUConvert %v4ushort [[shr]]
  uint16_t4 dr = reversebits(d);


// CHECK:      [[e:%[0-9]+]] = OpLoad %ulong %e
// CHECK-NEXT: [[uconv1:%[0-9]+]] = OpUConvert %uint [[e]]
// CHECK-NEXT: [[shr:%[0-9]+]] = OpShiftRightLogical %ulong [[e]] %ulong_32
// CHECK-NEXT: [[uconv2:%[0-9]+]] = OpUConvert %uint [[shr]]
// CHECK-NEXT: [[rev1:%[0-9]+]] = OpBitReverse %uint [[uconv1]]
// CHECK-NEXT: [[rev2:%[0-9]+]] = OpBitReverse %uint [[uconv2]]
// CHECK-NEXT: [[uconv3:%[0-9]+]] = OpUConvert %ulong [[rev1]]
// CHECK-NEXT: [[uconv4:%[0-9]+]] = OpUConvert %ulong [[rev2]]
// CHECK-NEXT: [[shl:%[0-9]+]] = OpShiftLeftLogical %ulong [[uconv3]] %ulong_32
// CHECK-NEXT: {{%[0-9]+}} = OpBitwiseOr %ulong [[shl]] [[uconv4]]
  uint64_t er = reversebits(e);

// CHECK:      [[f:%[0-9]+]] = OpLoad %v4ulong %f
// CHECK-NEXT: [[uconv1:%[0-9]+]] = OpUConvert %v4uint [[f]]
// CHECK-NEXT: [[shr:%[0-9]+]] = OpShiftRightLogical %v4ulong [[f]] [[v4ulong_32]]
// CHECK-NEXT: [[uconv2:%[0-9]+]] = OpUConvert %v4uint [[shr]]
// CHECK-NEXT: [[rev1:%[0-9]+]] = OpBitReverse %v4uint [[uconv1]]
// CHECK-NEXT: [[rev2:%[0-9]+]] = OpBitReverse %v4uint [[uconv2]]
// CHECK-NEXT: [[uconv3:%[0-9]+]] = OpUConvert %v4ulong [[rev1]]
// CHECK-NEXT: [[uconv4:%[0-9]+]] = OpUConvert %v4ulong [[rev2]]
// CHECK-NEXT: [[shl:%[0-9]+]] = OpShiftLeftLogical %v4ulong [[uconv3]] [[v4ulong_32]]
// CHECK-NEXT: {{%[0-9]+}} = OpBitwiseOr %v4ulong [[shl]] [[uconv4]]
  uint64_t4 fr = reversebits(f);
}
