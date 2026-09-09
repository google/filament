// RUN: %dxc -E main -T ps_6_0 -Zi -Od %s | FileCheck %s

// CHECK: @main

RWTexture1D<float> uav0 : register(u0);
RWTexture1D<float> uav1 : register(u1);

[RootSignature("DescriptorTable(UAV(u0,numDescriptors=10))")]
float main() : SV_Target {
    float ret = 0;
    // CHECK: call void @llvm.dbg.value(metadata float 0.000000e+00, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret"

    // CHECK-DAG: %[[sin_0:.+]] = call float @dx.op.unary.f32(i32 13,
    // CHECK-DAG: %[[cos_0:.+]] = call float @dx.op.unary.f32(i32 12,
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[sin_0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"a"
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[cos_0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"b"
    // CHECK-DAG: %[[add_0:.+]] = fadd fast float %[[sin_0]], %[[cos_0]]
    // CHECK: %[[add_add_0:.+]] = fadd fast float 0.000000e+00, %[[add_0]]

    // CHECK: call void @llvm.dbg.value(metadata float %[[add_add_0]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret"
    // CHECK-DAG: %[[sin_1:.+]] = call float @dx.op.unary.f32(i32 13,
    // CHECK-DAG: %[[cos_1:.+]] = call float @dx.op.unary.f32(i32 12,
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[sin_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"a"
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[cos_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"b"
    // CHECK-DAG: %[[add_1:.+]] = fadd fast float %[[sin_1]], %[[cos_1]]
    // CHECK: %[[add_add_1:.+]] = fadd fast float %[[add_add_0]], %[[add_1]]

    // CHECK: call void @llvm.dbg.value(metadata float %[[add_add_1]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret"
    // CHECK-DAG: %[[sin_2:.+]] = call float @dx.op.unary.f32(i32 13,
    // CHECK-DAG: %[[cos_2:.+]] = call float @dx.op.unary.f32(i32 12,
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[sin_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"a"
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[cos_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"b"
    // CHECK-DAG: %[[add_2:.+]] = fadd fast float %[[sin_2]], %[[cos_2]]
    // CHECK: %[[add_add_2:.+]] = fadd fast float %[[add_add_1]], %[[add_2]]

    // CHECK: call void @llvm.dbg.value(metadata float %[[add_add_2]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ret"
    // CHECK-DAG: %[[sin_3:.+]] = call float @dx.op.unary.f32(i32 13,
    // CHECK-DAG: %[[cos_3:.+]] = call float @dx.op.unary.f32(i32 12,
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[sin_3]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"a"
    // CHECK-DAG: call void @llvm.dbg.value(metadata float %[[cos_3]], i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"b"
    // CHECK-DAG: %[[add_3:.+]] = fadd fast float %[[sin_3]], %[[cos_3]]
    // CHECK: %[[add_add_3:.+]] = fadd fast float %[[add_add_2]], %[[add_3]]

    // CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float %{{.+}}), !dbg !{{[0-9]+}}

    [unroll]
    for (int i = 0; i < 4; i++) {
        float a = sin(uav0[i]);
        float b = cos(uav1[i]);
        ret += a + b;
    }
    return ret;
}

