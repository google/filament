// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main(out uint  a : A, out uint2  b : B, out uint3  c : C, out uint4  d : D,
              float m : M,     float2 n : N,     float3 o : O,     float4 p : P) {
// CHECK:        [[m:%[0-9]+]] = OpLoad %float %m
// CHECK-NEXT: [[vec:%[0-9]+]] = OpCompositeConstruct %v2float [[m]] %float_0
// CHECK-NEXT:   [[a:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec]]
// CHECK-NEXT:                OpStore %a [[a]]
    a = f32tof16(m);

// CHECK-NEXT:   [[n:%[0-9]+]] = OpLoad %v2float %n

// CHECK-NEXT:  [[n0:%[0-9]+]] = OpCompositeExtract %float [[n]] 0
// CHECK-NEXT: [[vec_0:%[0-9]+]] = OpCompositeConstruct %v2float [[n0]] %float_0
// CHECK-NEXT:  [[b0:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_0]]

// CHECK-NEXT:  [[n1:%[0-9]+]] = OpCompositeExtract %float [[n]] 1
// CHECK-NEXT: [[vec_1:%[0-9]+]] = OpCompositeConstruct %v2float [[n1]] %float_0
// CHECK-NEXT:  [[b1:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_1]]

// CHECK-NEXT:   [[b:%[0-9]+]] = OpCompositeConstruct %v2uint [[b0]] [[b1]]
// CHECK-NEXT:                OpStore %b [[b]]
    b = f32tof16(n);

// CHECK-NEXT:   [[o:%[0-9]+]] = OpLoad %v3float %o

// CHECK-NEXT:  [[o0:%[0-9]+]] = OpCompositeExtract %float [[o]] 0
// CHECK-NEXT: [[vec_2:%[0-9]+]] = OpCompositeConstruct %v2float [[o0]] %float_0
// CHECK-NEXT:  [[c0:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_2]]

// CHECK-NEXT:  [[o1:%[0-9]+]] = OpCompositeExtract %float [[o]] 1
// CHECK-NEXT: [[vec_3:%[0-9]+]] = OpCompositeConstruct %v2float [[o1]] %float_0
// CHECK-NEXT:  [[c1:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_3]]

// CHECK-NEXT:  [[o2:%[0-9]+]] = OpCompositeExtract %float [[o]] 2
// CHECK-NEXT: [[vec_4:%[0-9]+]] = OpCompositeConstruct %v2float [[o2]] %float_0
// CHECK-NEXT:  [[c2:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_4]]

// CHECK-NEXT:   [[c:%[0-9]+]] = OpCompositeConstruct %v3uint [[c0]] [[c1]] [[c2]]
// CHECK-NEXT:                OpStore %c [[c]]
    c = f32tof16(o);

// CHECK-NEXT:   [[p:%[0-9]+]] = OpLoad %v4float %p

// CHECK-NEXT:  [[p0:%[0-9]+]] = OpCompositeExtract %float [[p]] 0
// CHECK-NEXT: [[vec_5:%[0-9]+]] = OpCompositeConstruct %v2float [[p0]] %float_0
// CHECK-NEXT:  [[d0:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_5]]

// CHECK-NEXT:  [[p1:%[0-9]+]] = OpCompositeExtract %float [[p]] 1
// CHECK-NEXT: [[vec_6:%[0-9]+]] = OpCompositeConstruct %v2float [[p1]] %float_0
// CHECK-NEXT:  [[d1:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_6]]

// CHECK-NEXT:  [[p2:%[0-9]+]] = OpCompositeExtract %float [[p]] 2
// CHECK-NEXT: [[vec_7:%[0-9]+]] = OpCompositeConstruct %v2float [[p2]] %float_0
// CHECK-NEXT:  [[d2:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_7]]

// CHECK-NEXT:  [[p3:%[0-9]+]] = OpCompositeExtract %float [[p]] 3
// CHECK-NEXT: [[vec_8:%[0-9]+]] = OpCompositeConstruct %v2float [[p3]] %float_0
// CHECK-NEXT:  [[d3:%[0-9]+]] = OpExtInst %uint [[glsl]] PackHalf2x16 [[vec_8]]

// CHECK-NEXT:   [[d:%[0-9]+]] = OpCompositeConstruct %v4uint [[d0]] [[d1]] [[d2]] [[d3]]
// CHECK-NEXT:                OpStore %d [[d]]
    d = f32tof16(p);
}
