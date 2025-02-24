// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 %s | FileCheck %s

// CHECK: %dx.types.CBufRet.f64 = type { double, double }
// CHECK: %dx.types.CBufRet.i64 = type { i64, i64 }
// CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 0)
// CHECK: call %dx.types.CBufRet.f64 @dx.op.cbufferLoadLegacy.f64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 1)
// CHECK: call %dx.types.CBufRet.i64 @dx.op.cbufferLoadLegacy.i64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 2)
// CHECK: call %dx.types.CBufRet.i64 @dx.op.cbufferLoadLegacy.i64(i32 59, %dx.types.Handle %Foo_cbuffer, i32 3)

cbuffer Foo {
  double4 d;
  int64_t4 i;
}

float4 main() : SV_Target {
  return d + i;
}