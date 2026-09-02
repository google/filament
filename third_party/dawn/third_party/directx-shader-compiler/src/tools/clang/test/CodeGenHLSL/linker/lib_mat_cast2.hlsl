// RUN: %dxc -T lib_6_3 -default-linkage external %s | FileCheck %s

// CHECK: bitcast %class.matrix.float.4.3* {{.*}} to <12 x float>*

float3 mat_test(in float4 in0,
                                  in float4 in1,
                                  inout float4x3 m)
{
uint basisIndex = in1.w;
float3 outputs = mul(in0.xyz,
            (float3x3)m);
return outputs + m[in1.z];
}