// RUN: %dxc -E main -T cs_6_0 %s -Od | FileCheck %s

RWBuffer<float> uav : register(u0);

cbuffer cb : register(b0) {
  float foo;
  uint i;
}

groupshared float bar;

[numthreads(1, 1, 1)]
[RootSignature("CBV(b0), DescriptorTable(UAV(u0))")]
void main() {

  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  // CHECK: store
  bar = 1;

  // CHECK: store
  bar = foo;

  // CHECK:  dx.nothing
  float ret = foo;

  // CHECK:  dx.nothing
  ret = bar;

  // CHECK: call void @dx.op.bufferStore.f32(
  uav[i] = ret;
}

