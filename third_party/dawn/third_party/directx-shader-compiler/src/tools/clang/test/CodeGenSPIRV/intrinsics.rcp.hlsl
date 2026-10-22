// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v4f1:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
// CHECK: [[v3f1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1
// CHECK: [[v4d1:%[0-9]+]] = OpConstantComposite %v4double %double_1 %double_1 %double_1 %double_1
// CHECK: [[v3d1:%[0-9]+]] = OpConstantComposite %v3double %double_1 %double_1 %double_1

void main() {
  float    a, rcpa;
  float4   b, rcpb;
  float2x3 c, rcpc;
  
  double    d, rcpd;
  double4   e, rcpe;
  double2x3 f, rcpf;

// CHECK:      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:   {{%[0-9]+}} = OpFDiv %float %float_1 [[a]]
  rcpa = rcp(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:   {{%[0-9]+}} = OpFDiv %v4float [[v4f1]] [[b]]
  rcpb = rcp(b);

// CHECK:          [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:    [[c0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT: [[rcpc0:%[0-9]+]] = OpFDiv %v3float [[v3f1]] [[c0]]
// CHECK-NEXT:    [[c1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT: [[rcpc1:%[0-9]+]] = OpFDiv %v3float [[v3f1]] [[c1]]
// CHECK-NEXT:       {{%[0-9]+}} = OpCompositeConstruct %mat2v3float [[rcpc0]] [[rcpc1]]
  rcpc = rcp(c);

// CHECK:      [[d:%[0-9]+]] = OpLoad %double %d
// CHECK-NEXT:   {{%[0-9]+}} = OpFDiv %double %double_1 [[d]]
  rcpd = rcp(d);  

// CHECK:    [[e:%[0-9]+]] = OpLoad %v4double %e
// CHECK-NEXT: {{%[0-9]+}} = OpFDiv %v4double [[v4d1]] [[e]]
  rcpe = rcp(e);

// CHECK:          [[f:%[0-9]+]] = OpLoad %mat2v3double %f
// CHECK-NEXT:    [[f0:%[0-9]+]] = OpCompositeExtract %v3double [[f]] 0
// CHECK-NEXT: [[rcpf0:%[0-9]+]] = OpFDiv %v3double [[v3d1]] [[f0]]
// CHECK-NEXT:    [[f1:%[0-9]+]] = OpCompositeExtract %v3double [[f]] 1
// CHECK-NEXT: [[rcpf1:%[0-9]+]] = OpFDiv %v3double [[v3d1]] [[f1]]
// CHECK-NEXT:       {{%[0-9]+}} = OpCompositeConstruct %mat2v3double [[rcpf0]] [[rcpf1]]
  rcpf = rcp(f);

// Case with literal float argument.
// CHECK:      [[one_plus_two:%[0-9]+]] = OpFAdd %float %float_1 %float_2
// CHECK-NEXT:              {{%[0-9]+}} = OpFDiv %float %float_1 [[one_plus_two]]
  float g = rcp(1.0 + 2.0);
}
