// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// Make sure that when the call is precise we do not use fast math flags
// on the floating point instructions and add precise metadata to the
// generated dxil calls.

// CHECK: [[X:%.*]]   = call float @dx.op.loadInput.f32(i32 4
// CHECK: [[r0:%.*]]  = fmul float [[X]], 0x3FF7154760000000
// CHECK: [[r1:%.*]]  = call float @dx.op.unary.f32(i32 21, float [[r0]]), !dx.precise
// CHECK: [[r2:%.*]]  = fsub float 0.000000e+00, [[r0]]
// CHECK: [[r3:%.*]]  = call float @dx.op.unary.f32(i32 21, float [[r2]]), !dx.precise
// CHECK: [[r4:%.*]]  = fsub float [[r1]], [[r3]]
// CHECK: [[r5:%.*]]  = fadd float [[r1]], [[r3]]
// CHECK: fdiv float [[r4]], [[r5]]

[RootSignature("")]
precise float main(float x : A) : SV_Target {
    return tanh(x);
}