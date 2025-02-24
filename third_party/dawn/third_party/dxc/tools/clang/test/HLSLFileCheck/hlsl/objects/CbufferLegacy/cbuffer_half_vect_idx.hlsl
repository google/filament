// RUN: %dxc -E main -T ps_6_2 -enable-16bit-types %s | FileCheck %s

// CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %constants_cbuffer, i32 0)
// CHECK: extractvalue %dx.types.CBufRet.f16.8 %{{[^,]+}}, 0
// CHECK: extractvalue %dx.types.CBufRet.f16.8 %{{[^,]+}}, 1
// CHECK: call %dx.types.CBufRet.f16.8 @dx.op.cbufferLoadLegacy.f16(i32 59, %dx.types.Handle %constants_cbuffer, i32 1)
// CHECK: extractvalue %dx.types.CBufRet.f16.8 %{{[^,]+}}, 6
// CHECK: extractvalue %dx.types.CBufRet.f16.8 %{{[^,]+}}, 7

cbuffer constants : register(b0)
{
    half2 h2_1;
    float3 f3_1;
    float3 f3_2;
    half2 h2_2;
}

float main() : SV_TARGET
{
    half res1 = h2_1[0] + h2_1[1];
    half res2 = h2_2[0] + h2_2[1];
    float f = f3_2[1];
    return res1 + res2 + f;
}
