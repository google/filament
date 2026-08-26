// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// CHECK: [[X:%.*]]   = call float @dx.op.loadInput.f32(i32 4
// CHECK: [[r0:%.*]]  = call float @dx.op.unary.f32(i32 6, float [[X]]

// CHECK: [[b0:%.*]]  = fcmp fast ugt float [[r0]], 1.000000e+00
// CHECK: [[r1:%.*]]  = fdiv fast float 1.000000e+00, [[r0]]
// CHECK: [[r2:%.*]]  = select i1 [[b0]], float [[r1]], float [[r0]]

// CHECK: [[r3:%.*]]  = fmul fast float [[r2]],  [[r2]]
// CHECK: [[r4a:%.*]] = fmul fast float [[r3]],  0x3F9555CBE0000000
// CHECK: [[r4b:%.*]] = fadd fast float [[r4a]], 0xBFB5CB46C0000000 
// CHECK: [[r4c:%.*]] = fmul fast float [[r4b]], [[r3]]
// CHECK: [[r4d:%.*]] = fadd fast float [[r4c]], 0x3FC70EDC40000000
// CHECK: [[r4e:%.*]] = fmul fast float [[r4d]], [[r3]]
// CHECK: [[r4f:%.*]] = fadd fast float [[r4e]], 0xBFD523A080000000
// CHECK: [[r4g:%.*]] = fmul fast float [[r4f]], [[r3]]
// CHECK: [[r4h:%.*]] = fadd fast float [[r4g]], 0x3FEFFEE700000000
// CHECK: [[r4:%.*]]  = fmul fast float [[r2]],  [[r4h]]

// CHECK: [[r5:%.*]]  = fsub fast float 0x3FF921FB60000000, [[r4]]
// CHECK: [[r6:%.*]]  = select i1 [[b0]], float [[r5]], float [[r4]]

// CHECK: [[r7:%.*]]  = fsub fast float 0.000000e+00, [[r6]]

// CHECK: [[b1:%.*]]  = fcmp fast ult float [[X]], 0.000000e+00
// CHECK: select i1 [[b1]], float [[r7]], float [[r6]]


// CHECK-NOT: call float @dx.op.unary.f32(i32 17

[RootSignature("")]
float main(float x : A) : SV_Target {
    return atan(x);
}