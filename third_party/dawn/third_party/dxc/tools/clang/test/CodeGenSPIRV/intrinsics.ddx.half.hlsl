// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

void main() {

  half    a;
  half2   b;

// CHECK:      [[a:%[0-9]+]] = OpLoad %half %a
// CHECK-NEXT: [[c:%[0-9]+]] = OpFConvert %float [[a]]
// CHECK-NEXT:   [[r:%[0-9]+]] = OpDPdx %float [[c]]
// CHECK-NEXT:  OpFConvert %half [[r]]
  half    da = ddx(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v2half %b
// CHECK-NEXT: [[c:%[0-9]+]] = OpFConvert %v2float [[b]]
// CHECK-NEXT: [[r:%[0-9]+]] = OpDPdx %v2float [[c]]
// CHECK-NEXT:  OpFConvert %v2half [[r]]
  half2   db = ddx(b);
}
