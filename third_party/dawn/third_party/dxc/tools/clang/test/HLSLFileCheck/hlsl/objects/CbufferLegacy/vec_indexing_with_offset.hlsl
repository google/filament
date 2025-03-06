// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure store yzw to float3.
// CHECK:%[[X:.*]] = extractvalue %dx.types.CBufRet.f32 %{{.*}}, 1
// CHECK:%[[Y:.*]] = extractvalue %dx.types.CBufRet.f32 %{{.*}}, 2
// CHECK:%[[Z:.*]] = extractvalue %dx.types.CBufRet.f32 %{{.*}}, 3
// CHECK:%[[PX:.*]] = getelementptr inbounds [3 x float], [3 x float]* %{{.*}}, i32 0, i32 0
// CHECK:store float %[[X]], float* %[[PX]], align 4
// CHECK:%[[PY:.*]] = getelementptr inbounds [3 x float], [3 x float]* %{{.*}}, i32 0, i32 1
// CHECK:store float %[[Y]], float* %[[PY]], align 4
// CHECK:%[[PZ:.*]] = getelementptr inbounds [3 x float], [3 x float]* %{{.*}}, i32 0, i32 2
// CHECK:store float %[[Z]], float* %[[PZ]], align 4

cbuffer cb
{
    uint i;
    float3 v;
};

float4 main() : SV_TARGET
{
    return v[i];
}