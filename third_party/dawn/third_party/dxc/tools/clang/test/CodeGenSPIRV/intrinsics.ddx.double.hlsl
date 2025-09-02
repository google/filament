// RUN: %dxc -T ps_6_2 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

// CHECK: :14:22: warning: conversion from larger type 'double' to smaller type 'float', possible loss of data [-Wconversion]
// CHECK: :20:22: warning: conversion from larger type 'double2' to smaller type 'vector<float, 2>', possible loss of data [-Wconversion]

void main() {
  double    a;
  double2   b;

// CHECK:      [[a:%[0-9]+]] = OpLoad %double %a
// CHECK-NEXT: [[c:%[0-9]+]] = OpFConvert %float [[a]]
// CHECK-NEXT:   [[r:%[0-9]+]] = OpDPdx %float [[c]]
// CHECK-NEXT:  OpFConvert %double [[r]]
  double    da = ddx(a);

// CHECK:      [[b:%[0-9]+]] = OpLoad %v2double %b
// CHECK-NEXT: [[c:%[0-9]+]] = OpFConvert %v2float [[b]]
// CHECK-NEXT: [[r:%[0-9]+]] = OpDPdx %v2float [[c]]
// CHECK-NEXT:  OpFConvert %v2double [[r]]
  double2   db = ddx(b);
}