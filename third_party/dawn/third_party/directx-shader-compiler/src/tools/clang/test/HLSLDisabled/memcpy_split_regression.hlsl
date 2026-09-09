// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s
// 38575954: ARM64-only compiler crash in memcpy_split_regression.hlsl and memcpy_split_regression-strictudt.hlsl

//
// CHECK: @main
//
// Regression test for certain allocas not being split by SROA because
// SROA_Helper::LowerMemcpy returned true but didn't actually remove all uses
// of the alloca.
//
// Consider the following example:
// 
//     a = alloca MyStruct
//     b = alloca MyStruct
//     c = alloca MyStruct
//     ...
//     memcpy(b, a, sizeof(MyStruct))
//     ...
//     memcpy(c, b, sizeof(MyStruct))
//
// When SROA_Helper::LowerMemcpy is working on b, ReplaceMemcpy replaces the
// memcpy with loads and stores and returns true:
// 
//     a = alloca MyStruct
//     b = alloca MyStruct
//     c = alloca MyStruct
//     ...
//     b->member1 = a->member1;
//     b->member2 = a->member2;
//     ...
//     b->memberN = a->memberN;
//     ...
//     memcpy(c, b, sizeof(MyStruct))
//
// At this point SROA_Helper::LowerMemcpy returned true because it incorrectly
// assumed that all uses of `b` has been removed because it only has one store
// which is memcpy, and ReplaceMemcpy has replaced it, even though the alloca
// is still being used.
//

struct MyStruct {
  float4 foo;
  float4 bar;
  float4 baz;
};

cbuffer cb : register(b0) {
  float4 foo;
  float4 bar;
  float4 baz;
};

MyStruct make() {
  MyStruct local;
  local.foo = foo;
  local.bar = bar;
  local.baz = baz;
  return local;
}

MyStruct transform_mul(MyStruct arg_0) {
  MyStruct st[1] = {{
    arg_0.foo * 2,
    arg_0.bar * 2,
    arg_0.baz * 2,
  }};
  return arg_0;
}

MyStruct transform(MyStruct arg) {
  MyStruct ret;
  ret = transform_mul(arg);
  return ret;
}

float4 main() : SV_Target {
  MyStruct v0 = (MyStruct)0;
  v0 = make();
  MyStruct v1[1] = {transform(v0)};
  MyStruct v2[1] = {transform(v1)};
  MyStruct v3[1] = {transform(v2)};
  MyStruct v4[1] = {transform(v3)};
  return v4[0].foo + v4[0].bar + v4[0].baz;
}


