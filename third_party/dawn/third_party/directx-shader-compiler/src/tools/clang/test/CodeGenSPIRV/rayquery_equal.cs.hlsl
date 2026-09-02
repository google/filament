// RUN: %dxc -T cs_6_5 -E main -fspv-target-env=vulkan1.2 -spirv %s | FileCheck %s

RaytracingAccelerationStructure g_topLevel : register(t0, space0);
RWTexture2D<float4> g_output : register(u1, space0);

[numthreads(64, 1, 1)]
void main(uint2 launchIndex: SV_DispatchThreadID)
{
    float3 T = (float3)0;
    float sampleCount = 0;
    RayDesc ray;

    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 1, 0);
    ray.TMin = 0.0;
    ray.TMax = 1000.0;

    RayQuery<RAY_FLAG_FORCE_OPAQUE> q;

    q.TraceRayInline(g_topLevel, 0, 0xff, ray);
// CHECK:  [[rayquery:%[0-9]+]] = OpVariable %_ptr_Function_rayQueryKHR Function
    q.Proceed();
// CHECK:  OpRayQueryProceedKHR %bool [[rayquery]]
    if(q.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
// CHECK:  [[status:%[0-9]+]] = OpRayQueryGetIntersectionTypeKHR %uint [[rayquery]] %uint_1
// CHECK:  OpIEqual %bool [[status]] %uint_1
    {
        T += float3(1, 0, 1);
    }
    else
    {
        T += float3(0, 1, 0);
    }

    g_output[launchIndex] += float4(T, 1);
}
