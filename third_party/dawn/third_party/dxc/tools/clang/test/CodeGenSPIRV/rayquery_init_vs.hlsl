// RUN: %dxc -T vs_6_5 -E main -fspv-target-env=vulkan1.2 -spirv %s | FileCheck %s

// CHECK:  OpCapability RayQueryKHR
// CHECK:  OpExtension "SPV_KHR_ray_query"

RaytracingAccelerationStructure AccelerationStructure : register(t0);
RWByteAddressBuffer log : register(u0);

RayDesc MakeRayDesc()
{
    RayDesc desc;
    desc.Origin = float3(0,0,0);
    desc.Direction = float3(1,0,0);
    desc.TMin = 0.0f;
    desc.TMax = 9999.0;
    return desc;
}


void DoSomething()
{
    log.Store(0,1);
}

void doInitialize(RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> query, RayDesc ray)
{
    query.TraceRayInline(AccelerationStructure,RAY_FLAG_NONE,0xFF,ray);
}


void main()
{
// CHECK:  %rayQueryKHR = OpTypeRayQueryKHR
// CHECK:  %_ptr_Function_rayQueryKHR = OpTypePointer Function %rayQueryKHR
// CHECK:  [[rayquery:%[0-9]+]] = OpVariable %_ptr_Function_rayQueryKHR Function
    RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    RayDesc ray = MakeRayDesc();
// CHECK:  [[accel:%[0-9]+]] = OpLoad %accelerationStructureNV %AccelerationStructure
// CHECK:  OpRayQueryInitializeKHR [[rayquery]] [[accel]] %uint_517 %uint_255 {{%[0-9]+}} %float_0 {{%[0-9]+}} %float_9999

    q.TraceRayInline(AccelerationStructure,RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES, 0xFF, ray);
// CHECK: OpRayQueryInitializeKHR [[rayquery]] [[accel]] %uint_5 %uint_255 {{%[0-9]+}} %float_0 {{%[0-9]+}} %float_9999
    doInitialize(q, ray);
}
