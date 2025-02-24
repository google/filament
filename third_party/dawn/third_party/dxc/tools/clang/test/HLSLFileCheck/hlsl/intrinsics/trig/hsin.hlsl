// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// CHECK: [[X:%.*]]   = call float @dx.op.loadInput.f32(i32 4
// CHECK: [[r0:%.*]]  = fmul fast float [[X]], 0x3FF7154760000000
// CHECK: [[r1:%.*]]  = call float @dx.op.unary.f32(i32 21, float [[r0]]
// CHECK: [[r2:%.*]]  = fsub fast float 0.000000e+00, [[r0]]
// CHECK: [[r3:%.*]]  = call float @dx.op.unary.f32(i32 21, float [[r2]]
// CHECK: [[r4:%.*]]  = fsub fast float [[r1]], [[r3]]
// CHECK: fdiv fast float [[r4]], 2.000000e+00

// CHECK-NOT: call float @dx.op.unary.f32(i32 18

[RootSignature("")]
float main(float x : A) : SV_Target {
    return sinh(x);
}