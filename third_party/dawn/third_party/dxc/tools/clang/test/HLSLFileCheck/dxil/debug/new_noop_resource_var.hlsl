// RUN: %dxc -E main -T cs_6_0 %s -Od | FileCheck %s

RWBuffer<float> uav : register(u0);

cbuffer cb : register(b0) {
  float foo;
  uint i;
}

static RWBuffer<float> my_uav;

void store_things() {
  float val = sin(foo);
  RWBuffer<float> local_uav = my_uav;
  local_uav = my_uav;
  local_uav[i] = val;
}

[numthreads(1, 1, 1)]
[RootSignature("CBV(b0), DescriptorTable(UAV(u0))")]
void main() {
  // xHECK: %[[p_load:[0-9]+]] = load i32, i32*
  // xHECK-SAME: @dx.preserve.value
  // xHECK: %[[p:[0-9]+]] = trunc i32 %[[p_load]] to i1

  // CHECK: load i32, i32*
  // CHECK: @dx.nothing
  my_uav = uav;

  // select i1 [[p]],
  // CHECK: dx.nothing
  float ret = foo;

  // CHECK: load i32, i32*
  // CHECK: @dx.nothing
  store_things();
    // CHECK: unary.f32(i32 13
    // CHECK: load i32, i32*
    // CHECK: @dx.nothing
    // CHECK: load i32, i32*
    // CHECK: @dx.nothing
    // CHECK: call void @dx.op.bufferStore.f32(

  // CHECK: call void @dx.op.bufferStore.f32(
  uav[i] = ret;
}

