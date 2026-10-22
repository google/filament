// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// CHECK: define <4 x float>
// CHECK: fn1
// @"\01?fn1{{[@$?.A-Za-z0-9_]+}}"
// CHECK: #0
// CHECK: attributes #0
// CHECK: "exp-foo"="bar"
// CHECK: "exp-zzz"

[experimental("foo", "bar")]
[experimental("zzz", "")]
float4 fn1(float4 Input){
  return Input * 5.0f;
}
