// RUN: %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: [9 x half] [half 0xH3C00, half 0xH4400, half 0xH4700, half 0xH4000, half 0xH4500, half 0xH4800, half 0xH4200, half 0xH4600, half 0xH4880]
// CHECK: fptoui float
// CHECK: getelementptr [9 x half]
// CHECK: load half
// CHECK: call half @dx.op.tertiary.f16(i32 46, half 0xH4000,
// CHECK: lshr i32 411,
// CHECK: icmp ne
// CHECK: %[[all:[^ ]*]] = uitofp i1 %{{.*}} to float
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 2,
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 3, float %[[all]])

struct Foo {
    half3x3 hmat;
    bool3x3 bmat;
};

Foo fn() {
    Foo foo;
    foo.hmat = float3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
    foo.bmat = int3x3(1, 2, 0, -5, 14, 3, 0, 0, 21);
    return foo;
}

float4 main(float a : A) : SV_Target {
    Foo foo = fn();
    float3 v = float3(a, a * a, a + a);
    return float4(mul(foo.hmat, foo.hmat[a]), all(foo.bmat[a]));
}
