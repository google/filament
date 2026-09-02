// RUN: %dxilver 1.2 | %dxc -E main -T ps_6_2 -HV 2018 -enable-16bit-types %s | FileCheck %s

// CHECK: bitcast half %{{.*}} to i16
// CHECK: bitcast i16 %{{.*}} to half

float16_t g_h4;
int16_t4 g_i4;
uint16_t4 g_u4;

float4 main(float4 col : COL) : SV_TARGET
{
    float16_t4 h1 = asfloat16(g_h4);
    uint16_t4 u1 = asuint16(g_h4);
    int16_t4 i1 = asint16(g_h4);

    h1 += asfloat16(g_i4);
    u1 += asuint16(g_i4);
    i1 += asint16(g_i4);

    h1 += asfloat16(g_u4);
    u1 += asuint16(g_u4);
    i1 += asint16(g_u4);

    return h1 + u1 + i1;
}