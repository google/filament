// RUN: %dxc -T cs_6_0 -E main -O0 -spirv %s | FileCheck %s

// CHECK:       [[ext:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

[numthreads(1,1,1)]
void main() {
// CHECK:       [[foo:%[0-9]+]] = OpLoad %v4uint %foo
// CHECK-NEXT:    [[i:%[0-9]+]] = OpCompositeExtract %uint [[foo]] 0
// CHECK-NEXT: [[half:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[i]]
// CHECK-NEXT:    [[x:%[0-9]+]] = OpCompositeExtract %float [[half]] 0
// CHECK-NEXT:    [[i:%[0-9]+]] = OpCompositeExtract %uint [[foo]] 1
// CHECK-NEXT: [[half:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[i]]
// CHECK-NEXT:    [[y:%[0-9]+]] = OpCompositeExtract %float [[half]] 0
// CHECK-NEXT:    [[i:%[0-9]+]] = OpCompositeExtract %uint [[foo]] 2
// CHECK-NEXT: [[half:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[i]]
// CHECK-NEXT:    [[z:%[0-9]+]] = OpCompositeExtract %float [[half]] 0
// CHECK-NEXT:    [[i:%[0-9]+]] = OpCompositeExtract %uint [[foo]] 3
// CHECK-NEXT: [[half:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[i]]
// CHECK-NEXT:    [[w:%[0-9]+]] = OpCompositeExtract %float [[half]] 0
// CHECK-NEXT:  [[bar:%[0-9]+]] = OpCompositeConstruct %v4float [[x]] [[y]] [[z]] [[w]]
// CHECK-NEXT:                 OpStore %bar [[bar]]
  uint4 foo;
  half4 bar = half4(f16tof32(foo));
}
