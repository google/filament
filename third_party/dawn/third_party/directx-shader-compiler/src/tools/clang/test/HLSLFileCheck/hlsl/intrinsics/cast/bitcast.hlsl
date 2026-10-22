// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: bitcast float %{{.*}} to i32
// CHECK: bitcast i32 %{{.*}} to float

float4 g_f4;
int4   g_i4;
uint4  g_u4;

float4 main() : SV_Target
{
    float4 f1 = asfloat(g_f4);
    uint4 u1 = asuint(g_f4);
    int4 i1 = asint(g_f4);

    f1 += asfloat(g_i4);
    u1 += asuint(g_i4);
    i1 += asint(g_i4);

    f1 += asfloat(g_u4);
    u1 += asuint(g_u4);
    i1 += asint(g_u4);

    return f1 + u1 + i1;
}
