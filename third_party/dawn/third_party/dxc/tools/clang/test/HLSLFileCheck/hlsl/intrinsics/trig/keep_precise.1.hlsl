// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// Make sure precise->non-precise->precise transition is handled properly.

// A
// CHECK: fmul float {{.*}}, 0x3FF7154760000000
// CHECK: call float @dx.op.unary.f32(i32 21, float {{.*}}), !dx.precise
// CHECK: call float @dx.op.unary.f32(i32 21, float {{.*}}), !dx.precise

// B
// CHECK: fmul fast float {{.*}}, 0x3FF7154760000000
// CHECK: call float @dx.op.unary.f32(i32 21, float {{.*}})
// CHECK-NOT: !dx.precise
// CHECK: call float @dx.op.unary.f32(i32 21, float {{.*}})
// CHECK-NOT: !dx.precise

// C
// CHECK: fmul float {{.*}}, 0x3FF7154760000000
// CHECK: call float @dx.op.unary.f32(i32 21, float {{.*}}), !dx.precise
// CHECK: call float @dx.op.unary.f32(i32 21, float {{.*}}), !dx.precise

// CHECK: ret

[RootSignature("")]
float main(float x : A, float y : B, float z : C) : SV_Target {
    precise float a = tanh(x);
            float b = tanh(y);
    precise float c = tanh(z);
    return a + b + c;
}