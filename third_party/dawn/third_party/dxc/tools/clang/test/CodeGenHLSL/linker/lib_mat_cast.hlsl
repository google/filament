// RUN: %dxc -T lib_6_3 -default-linkage external %s | FileCheck %s

// CHECK: bitcast %class.matrix.float.4.3* {{.*}} to <12 x float>*

float mat_array_test(in float4 in0,
                     in float4 in1,
                     float4x3 basisArray[2]) // cube map basis.
{
  uint index = in1.w;

  float3 outputs = mul(in0.xyz, basisArray[index]);
  return outputs.z + basisArray[in1.y][in1.z].y;
}