// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// CHECK: [[X:%.*]]   = call float @dx.op.loadInput.f32(i32 4
// CHECK: [[r0:%.*]]  = call float @dx.op.unary.f32(i32 6, float [[X]]

// CHECK: [[r1:%.*]]  = fsub fast float 1.000000e+00, [[r0]]
// CHECK: [[r2:%.*]]  = call float @dx.op.unary.f32(i32 24, float [[r1]]

// CHECK: [[r3a:%.*]] = fmul fast float [[r0]], 0xBF932DC600000000
// CHECK: [[r3b:%.*]] = fadd fast float [[r3a]], 0x3FB302C4E0000000
// CHECK: [[r3c:%.*]] = fmul fast float [[r0]], [[r3b]]
// CHECK: [[r3d:%.*]] = fadd fast float [[r3c]], 0xBFCB269080000000
// CHECK: [[r3e:%.*]] = fmul fast float [[r0]], [[r3d]]
// CHECK: [[r3f:%.*]] = fadd fast float [[r3e]], 0x3FF921B480000000
// CHECK: [[r4:%.*]]  = fmul fast float [[r2]], [[r3f]]

// CHECK: [[r5:%.*]]  = fsub fast float 0x3FF921FB60000000, [[r4]]
// CHECK: [[r6:%.*]]  = fsub fast float 0.000000e+00, [[r5]]

// CHECK: [[b0:%.*]]  = fcmp fast ult float [[X]], 0.000000e+00
// CHECK: select i1 [[b0]], float [[r6]], float [[r5]]

// CHECK-NOT: call float @dx.op.unary.f32(i32 16

[RootSignature("")]
float main(float x : A) : SV_Target {
    return asin(x);
}