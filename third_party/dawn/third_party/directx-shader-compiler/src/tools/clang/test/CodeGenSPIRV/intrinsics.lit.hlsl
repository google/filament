// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// The signature for 'lit' intrinsic function is as follows:
// float4 lit(float n_dot_l, float n_dot_h, float m)

// CHECK: [[glsl:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
  float n_dot_l, n_dot_h, m;
  
// CHECK:      [[n_dot_l:%[0-9]+]] = OpLoad %float %n_dot_l
// CHECK-NEXT: [[n_dot_h:%[0-9]+]] = OpLoad %float %n_dot_h
// CHECK-NEXT:       [[m:%[0-9]+]] = OpLoad %float %m
// CHECK-NEXT: [[diffuse:%[0-9]+]] = OpExtInst %float [[glsl]] FMax %float_0 [[n_dot_l]]
// CHECK-NEXT:     [[min:%[0-9]+]] = OpExtInst %float [[glsl]] FMin [[n_dot_l]] [[n_dot_h]]
// CHECK-NEXT:  [[is_neg:%[0-9]+]] = OpFOrdLessThan %bool [[min]] %float_0
// CHECK-NEXT:     [[mul:%[0-9]+]] = OpFMul %float [[n_dot_h]] [[m]]
// CHECK-NEXT:[[specular:%[0-9]+]] = OpSelect %float [[is_neg]] %float_0 [[mul]]
// CHECK-NEXT:         {{%[0-9]+}} = OpCompositeConstruct %v4float %float_1 [[diffuse]] [[specular]] %float_1
  float4 result = lit(n_dot_l, n_dot_h, m);
}
