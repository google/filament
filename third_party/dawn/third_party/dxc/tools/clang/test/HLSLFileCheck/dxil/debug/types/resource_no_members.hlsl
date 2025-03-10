// RUN: %dxc -T vs_6_0 -E main -Zi %s | FileCheck %s

// Test that the debug info for resources does not expose internal fields.
// Does not test the size of resources.

// CHECK-DAG: ![[empty:.*]] = !{}
// CHECK-DAG: !DICompositeType(tag: DW_TAG_class_type, name: "Texture1D<float>", {{.*}}, elements: ![[empty]]
// CHECK-DAG: !DICompositeType(tag: DW_TAG_class_type, name: "Buffer<float>", {{.*}}, elements: ![[empty]]
// CHECK-DAG: !DICompositeType(tag: DW_TAG_class_type, name: "RWStructuredBuffer<float>", {{.*}}, elements: ![[empty]]

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

Texture1D<float> tex;
Buffer<float> buf;
RWStructuredBuffer<float> rwstructbuf;
float main() : OUT { return tex.Load(0) + buf[0] + rwstructbuf[0]; }