// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zi %s | FileCheck %s

// Check that the clang field offsets (as seen from debug type definitions)
// and the LLVM field offsets (as seen from SROA'd offsets) are consistent
// when empty structs are involved.

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: !DICompositeType(tag: DW_TAG_structure_type, {{.*}}, size: 64, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "x", {{.*}}, align: 32)
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "empty1", {{.*}}, align: 8, offset: 32)
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "empty2", {{.*}}, align: 8, offset: 32)
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "y", {{.*}}, size: 32, align: 32, offset: 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 32, 32)

float main(float val : IN) : OUT
{
  struct
  {
    float x; // HLSL [0,32), C++ [0, 32)
    struct {} empty1; // HLSL [32,32), C++ [32, 40)
    struct {} empty2; // HLSL [32,32), C++ [40, 48)
    float y; // HLSL [32,64), C++ [64, 96)
  } s = { 0, val };
  return s.y;
}