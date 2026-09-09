// RUN: %dxc -T vs_6_0 -E main -Od -Zi %s | FileCheck %s

// Test that the debug info for HLSL bool matrices exposes per-component fields
// in the memory representation of matrices (32-bits)

// CHECK-DAG: !DIDerivedType(tag: DW_TAG_typedef, name: "bool2x2"
// CHECK-DAG: !DICompositeType(tag: DW_TAG_class_type, name: "matrix<bool, 2, 2>", {{.*}}, size: 128, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "_11", {{.*}}, size: 32, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "_12", {{.*}}, size: 32, align: 32, offset: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "_21", {{.*}}, size: 32, align: 32, offset: 64
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "_22", {{.*}}, size: 32, align: 32, offset: 96

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}
bool2x2 main(bool2x2 v : IN) : OUT { return v; }