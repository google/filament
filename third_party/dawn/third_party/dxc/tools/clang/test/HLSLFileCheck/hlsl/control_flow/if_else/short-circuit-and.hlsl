// RUN: %dxc /T ps_6_0 %s -HV 2021 -Zi | FileCheck %s
// RUN: %dxc /T ps_6_0 %s -HV 2018 | FileCheck %s -check-prefix=NO_SHORT_CIRCUIT

// Load the two uav handles
// CHECK-DAG: %[[uav_foo:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 128, i1 false)
// CHECK-DAG: %[[uav_bar:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 256, i1 false)

// First side effect
// CHECK-DAG: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %[[uav_foo]]

// LHS: sin(...) != 0
// CHECK-DAG: %[[sin:.+]] = call float @dx.op.unary.f32(i32 13
// CHECK: %[[foo_cmp:.+]] = fcmp fast une float %[[sin]]
// CHECK: br i1 %[[foo_cmp]], label %[[true_label:.+]], label %[[false_label:.+]],

// For AND, if first operand is TRUE, goes to evaluate the second operand
// CHECK: [[true_label]]{{:? *}}; preds =

// Second side effect
// CHECK: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %[[uav_bar]]

// RHS: cos(...) != 0
// CHECK-DAG: %[[cos:.+]] = call float @dx.op.unary.f32(i32 12
// CHECK: %[[bar_cmp:.+]] = fcmp fast une float %[[cos]]
// CHECK: br i1 %[[bar_cmp]]
// CHECK-SAME: %[[false_label]]

// Just check there's no branches.
// NO_SHORT_CIRCUIT-NOT: br i1 %{{.+}}
// NO_SHORT_CIRCUIT-NOT: br label %{{.+}}

RWBuffer<float> buf_foo : register(u128);
RWBuffer<float> buf_bar : register(u256);
cbuffer cb : register(b0) {
  uint write_idx0;
  uint write_idx1;
  float foo_val;
  float bar_val;
  bool a, b;
}

bool foo() {
  buf_foo[write_idx0] = foo_val;
  return sin(foo_val) != 0;
}

bool bar() {
  buf_bar[write_idx1] = bar_val;
  return cos(bar_val) != 0;
}

[RootSignature("DescriptorTable(SRV(t0,numDescriptors=32)),DescriptorTable(CBV(b0,numDescriptors=32)),DescriptorTable(UAV(u0,numDescriptors=1000))")]
float main() : SV_Target {
  float ret = 0;
  if (foo() && bar()) {
    ret = 1;
  }
  return ret;
}
