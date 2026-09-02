// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main(uint      a : A,     uint2  b : B,     uint3  c : C,     uint4  d : D,
          out float m : M, out float2 n : N, out float3 o : O, out float4 p : P) {
// CHECK:        [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK-NEXT: [[cov:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[a]]
// CHECK-NEXT:   [[m:%[0-9]+]] = OpCompositeExtract %float [[cov]] 0
// CHECK-NEXT:                OpStore %m [[m]]
    m = f16tof32(a);

// CHECK:        [[b:%[0-9]+]] = OpLoad %v2uint %b

// CHECK-NEXT:  [[b0:%[0-9]+]] = OpCompositeExtract %uint [[b]] 0
// CHECK-NEXT: [[cov_0:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[b0]]
// CHECK-NEXT:  [[n0:%[0-9]+]] = OpCompositeExtract %float [[cov_0]] 0

// CHECK-NEXT:  [[b1:%[0-9]+]] = OpCompositeExtract %uint [[b]] 1
// CHECK-NEXT: [[cov_1:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[b1]]
// CHECK-NEXT:  [[n1:%[0-9]+]] = OpCompositeExtract %float [[cov_1]] 0

// CHECK-NEXT:   [[n:%[0-9]+]] = OpCompositeConstruct %v2float [[n0]] [[n1]]
// CHECK-NEXT:                OpStore %n [[n]]
    n = f16tof32(b);

// CHECK-NEXT:   [[c:%[0-9]+]] = OpLoad %v3uint %c

// CHECK-NEXT:  [[c0:%[0-9]+]] = OpCompositeExtract %uint [[c]] 0
// CHECK-NEXT: [[cov_2:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[c0]]
// CHECK-NEXT:  [[o0:%[0-9]+]] = OpCompositeExtract %float [[cov_2]] 0

// CHECK-NEXT:  [[c1:%[0-9]+]] = OpCompositeExtract %uint [[c]] 1
// CHECK-NEXT: [[cov_3:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[c1]]
// CHECK-NEXT:  [[o1:%[0-9]+]] = OpCompositeExtract %float [[cov_3]] 0

// CHECK-NEXT:  [[c2:%[0-9]+]] = OpCompositeExtract %uint [[c]] 2
// CHECK-NEXT: [[cov_4:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[c2]]
// CHECK-NEXT:  [[o2:%[0-9]+]] = OpCompositeExtract %float [[cov_4]] 0

// CHECK-NEXT:   [[o:%[0-9]+]] = OpCompositeConstruct %v3float [[o0]] [[o1]] [[o2]]
// CHECK-NEXT:                OpStore %o [[o]]
    o = f16tof32(c);

// CHECK-NEXT:   [[d:%[0-9]+]] = OpLoad %v4uint %d

// CHECK-NEXT:  [[d0:%[0-9]+]] = OpCompositeExtract %uint [[d]] 0
// CHECK-NEXT: [[cov_5:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[d0]]
// CHECK-NEXT:  [[p0:%[0-9]+]] = OpCompositeExtract %float [[cov_5]] 0

// CHECK-NEXT:  [[d1:%[0-9]+]] = OpCompositeExtract %uint [[d]] 1
// CHECK-NEXT: [[cov_6:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[d1]]
// CHECK-NEXT:  [[p1:%[0-9]+]] = OpCompositeExtract %float [[cov_6]] 0

// CHECK-NEXT:  [[d2:%[0-9]+]] = OpCompositeExtract %uint [[d]] 2
// CHECK-NEXT: [[cov_7:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[d2]]
// CHECK-NEXT:  [[p2:%[0-9]+]] = OpCompositeExtract %float [[cov_7]] 0

// CHECK-NEXT:  [[d3:%[0-9]+]] = OpCompositeExtract %uint [[d]] 3
// CHECK-NEXT: [[cov_8:%[0-9]+]] = OpExtInst %v2float [[glsl]] UnpackHalf2x16 [[d3]]
// CHECK-NEXT:  [[p3:%[0-9]+]] = OpCompositeExtract %float [[cov_8]] 0

// CHECK-NEXT:   [[p:%[0-9]+]] = OpCompositeConstruct %v4float [[p0]] [[p1]] [[p2]] [[p3]]
// CHECK-NEXT:                OpStore %p [[p]]
    p = f16tof32(d);
}
