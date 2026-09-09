// RUN: %dxc /T ps_6_0 %s -HV 2021 -Zi | FileCheck %s
// RUN: %dxc /T ps_6_0 %s -HV 2018 | FileCheck %s -check-prefix=NO_SHORT_CIRCUIT

// Load the two uav handles
// CHECK-DAG: %[[uav_foo:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 128, i1 false)
// CHECK-DAG: %[[uav_bar:.+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 1, i32 256, i1 false)

// Cond:
// CHECK-DAG: %[[tan:.+]] = call float @dx.op.unary.f32(i32 14
// CHECK: %[[cond_cmp:.+]] = fcmp fast une float %[[tan]], 0.000000e+00

// The actual select
// CHECK: br i1 %[[cond_cmp]], label %[[true_label:.+]], label %[[false_label:.+]],

// CHECK: [[true_label]]{{:? *}}; preds =
// First side effect
// CHECK-DAG: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %[[uav_foo]]
// CHECK-DAG: %[[sin:.+]] = call float @dx.op.unary.f32(i32 13
// CHECK: br label %[[final_block:.+]],

// CHECK: [[false_label]]{{:? *}}; preds =
// Second side effect
// CHECK-DAG: call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %[[uav_bar]]
// CHECK-DAG: %[[cos:.+]] = call float @dx.op.unary.f32(i32 12
// CHECK: br label %[[final_block]],

// CHECK: [[final_block]]{{:? *}}; preds =
// CHECK: phi float [ %[[sin]], %[[true_label]] ],  [ %[[cos]], %[[false_label]] ]

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
  float cond;
  bool a, b;
}

float foo() {
  buf_foo[write_idx0] = foo_val;
  return sin(foo_val);
}

float bar() {
  buf_bar[write_idx1] = bar_val;
  return cos(bar_val);
}

[RootSignature("DescriptorTable(SRV(t0,numDescriptors=32)),DescriptorTable(CBV(b0,numDescriptors=32)),DescriptorTable(UAV(u0,numDescriptors=1000))")]
float main() : SV_Target {
  return tan(cond) != 0 ? foo() : bar();
}

