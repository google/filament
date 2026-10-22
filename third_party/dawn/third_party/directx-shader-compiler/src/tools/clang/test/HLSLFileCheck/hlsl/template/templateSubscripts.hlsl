// RUN: %dxc -T ps_6_0 -HV 2021 %s | FileCheck %s

// Test applying the [] subscript operator in a templated function.
// With the side effect of testing passing matrices, arrays, and vectors as params.

template<typename T>
float4 subscript(T t0) {
  return t0[3];
}

// CHECK: define void @main
// CHECK: @dx.op.loadInput.f32
// CHECK: @dx.op.loadInput.f32
// CHECK: @dx.op.loadInput.f32
bool main(float scalar : A, float4 vec : B, float4x4 mat : C, float4 arr[6] : D) : SV_Target {

  return subscript(vec) + subscript(mat) + subscript(arr);

}





