// REQUIRES: dxil-1-8
// RUN: %dxc -T cs_6_8 -fcgl %s | FileCheck %s

// CHECK: %dx.types.ResourceProperties = type { i32, i32 }
// CHECK: !dx.typeAnnotations = !{![[TYPE_ANNOTATIONS:[0-9]+]]}
// CHECK: ![[TYPE_ANNOTATIONS]] = !{
// CHECK-SAME: void (%struct.RWByteAddressBuffer*)* @"\01?foo@@YAXURWByteAddressBuffer@@@Z", ![[FN_ANN:[0-9]+]]
// CHECK: ![[FN_ANN]] = !{!{{[0-9]+}}, ![[PARAM_ANN:[0-9]+]]}
// CHECK: ![[PARAM_ANN]] = !{i32 0, ![[FIELD_ANN:[0-9]+]], !{{[0-9]+}}}
// CHECK: ![[FIELD_ANN]] = !{
// CHECK-SAME: i32 10, %dx.types.ResourceProperties { i32 4107, i32 0 }

RWByteAddressBuffer OutBuff : register(u0);

void foo(RWByteAddressBuffer Buf) {
  Buf.Store(0, 42);
}

[numthreads(8, 1, 1)]
void main() {
  foo(OutBuff);
}
