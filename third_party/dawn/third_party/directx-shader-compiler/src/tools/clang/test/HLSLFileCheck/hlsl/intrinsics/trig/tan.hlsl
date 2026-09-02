// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// CHECK: [[r0:%.*]]  = call float @dx.op.unary.f32(i32 13
// CHECK: [[r1:%.*]]  = call float @dx.op.unary.f32(i32 12
// CHECK: fdiv fast float [[r0]], [[r1]]
// CHECK-NOT: call float @dx.op.unary.f32(i32 14

[RootSignature("")]
float main(float x : A) : SV_Target {
    return tan(x);
}