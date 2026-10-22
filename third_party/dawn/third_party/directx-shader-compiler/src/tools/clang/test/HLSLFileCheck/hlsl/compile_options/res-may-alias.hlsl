// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_7 %s | FileCheck %s -check-prefixes=CHECK,NOALIAS
// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_7 -res-may-alias %s | FileCheck %s -check-prefixes=CHECK,ALIAS
// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_0 %s | FileCheck %s -check-prefixes=CHECK,ALIAS
// RUN: %dxilver 1.7 | %dxc -E main -T ps_6_0 -res-may-alias %s | FileCheck %s -check-prefixes=CHECK,ALIAS
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 -validator-version 1.5 %s | FileCheck %s -check-prefixes=CHECK,ALIAS
// RUN: %dxilver 1.5 | %dxc -E main -T ps_6_0 -validator-version 1.5 -res-may-alias %s | FileCheck %s -check-prefixes=CHECK,ALIAS

// flag masked off when dxil version < 1.7, meaning resources may alias.
// CHECK: = !{void ()* @main, !"main", !{{.*}}, !{{.*}}, ![[flags:[0-9]+]]}
// ALIAS: ![[flags]] = !{i32 0, i64 16}
// NOALIAS: ![[flags]] = !{i32 0, i64 8589934608}

RWStructuredBuffer<float> FBUF;
RWStructuredBuffer<int> IBUF;

float4 main(float4 a : A) : SV_Target
{
    float f = FBUF[0];
    IBUF[a.w] = -27;
    return f + FBUF[0];
}
