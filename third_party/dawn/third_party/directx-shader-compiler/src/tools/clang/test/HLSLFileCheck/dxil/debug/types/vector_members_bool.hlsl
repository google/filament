// RUN: %dxc -T vs_6_0 -E main -Od -Zi %s | FileCheck %s

// Test that the debug info for HLSL bool vectors exposes per-component fields
// in the memory representation of bools (32-bits)

// CHECK-DAG: !DIDerivedType(tag: DW_TAG_typedef, name: "bool2"
// CHECK-DAG: !DICompositeType(tag: DW_TAG_class_type, name: "vector<bool, 2>", {{.*}}, size: 64, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "x", {{.*}}, size: 32, align: 32
// CHECK-DAG: !DIDerivedType(tag: DW_TAG_member, name: "y", {{.*}}, size: 32, align: 32, offset: 32

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}
bool2 main(bool2 v : IN) : OUT { return v; }