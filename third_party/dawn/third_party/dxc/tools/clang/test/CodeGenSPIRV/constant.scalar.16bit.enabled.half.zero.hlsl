// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: [[ext:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

void main() {
// CHECK:      [[a:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b:%[0-9]+]] = OpSelect %half [[a]] %half_0x1p_0 %half_0x0p_0
// CHECK-NEXT:              OpStore %b [[b]]
  bool a;
  half b = a;

// CHECK:      [[c:%[0-9]+]] = OpLoad %v2bool %c
// CHECK-NEXT: [[d:%[0-9]+]] = OpSelect %v2half [[c]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:              OpStore %d [[d]]
  bool2 c;
  half2 d = c;

// CHECK:      [[d_0:%[0-9]+]] = OpLoad %v2half %d
// CHECK-NEXT: [[e:%[0-9]+]] = OpExtInst %v2half [[ext]] FClamp [[d_0]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:              OpStore %e [[e]]
  half2 e = saturate(d);

// CHECK:      [[b_0:%[0-9]+]] = OpLoad %half %b
// CHECK-NEXT: [[f:%[0-9]+]] = OpExtInst %half [[ext]] FClamp [[b_0]] %half_0x0p_0 %half_0x1p_0
// CHECK-NEXT:              OpStore %f [[f]]
  half f = saturate(b);

// CHECK:      [[a_0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[x:%[0-9]+]] = OpSelect %float [[a_0]] %float_1 %float_0
// CHECK-NEXT: [[y:%[0-9]+]] = OpExtInst %float [[ext]] FClamp [[x]] %float_0 %float_1
// CHECK-NEXT: [[g:%[0-9]+]] = OpFConvert %half [[y]]
// CHECK-NEXT:              OpStore %g [[g]]
  half g = (half)saturate(a);

// CHECK:      [[h:%[0-9]+]] = OpLoad %v2int %h
// CHECK-NEXT: [[x_0:%[0-9]+]] = OpConvertSToF %v2float [[h]]
// CHECK-NEXT: [[y_0:%[0-9]+]] = OpExtInst %v2float [[ext]] FClamp [[x_0]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[i:%[0-9]+]] = OpFConvert %v2half [[y_0]]
// CHECK-NEXT:              OpStore %i [[i]]
  int2 h;
  half2 i = (half2)saturate(h);

// CHECK:      [[j:%[0-9]+]] = OpLoad %v2float %j
// CHECK-NEXT: [[x_1:%[0-9]+]] = OpExtInst %v2float [[ext]] FClamp [[j]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT: [[k:%[0-9]+]] = OpFConvert %v2half [[x_1]]
// CHECK-NEXT:              OpStore %k [[k]]
  float2 j;
  half2 k = (half2)saturate(j);
}
