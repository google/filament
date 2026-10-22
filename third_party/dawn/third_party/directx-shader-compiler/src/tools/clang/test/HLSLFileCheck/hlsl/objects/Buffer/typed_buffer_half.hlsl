// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -enable-16bit-types %s  | FileCheck %s

// CHECK: call void @dx.op.bufferStore.f16

RWBuffer<half4> g_tb: register(u1);

float4 main() : SV_Target {
    g_tb[3] = half4(1,2,3,4);
    return 1;
}