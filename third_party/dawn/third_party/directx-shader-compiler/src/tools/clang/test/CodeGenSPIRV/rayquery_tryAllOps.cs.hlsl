// RUN: %dxc -T cs_6_5 -E CS -fspv-target-env=vulkan1.2 -O0 -spirv %s | FileCheck %s

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

[numThreads(1,1,1)]
void CS()
{
    RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    RayDesc ray = MakeRayDesc();
    q.TraceRayInline(AccelerationStructure,RAY_FLAG_NONE,0xFF,ray);
    float4x3 mat4x3;
    float3x4 mat3x4;
// CHECK:  [[rayquery:%[0-9]+]] = OpVariable %_ptr_Function_rayQueryKHR Function
// CHECK:  OpRayQueryProceedKHR %bool [[rayquery]]
    while(q.Proceed())
    {
// CHECK:  OpRayQueryGetIntersectionTypeKHR %uint [[rayquery]] %uint_0
        switch(q.CandidateType())
        {
        case CANDIDATE_NON_OPAQUE_TRIANGLE:
            q.Abort();
// CHECK:  OpRayQueryGetIntersectionObjectToWorldKHR %mat4v3float [[rayquery]] %uint_0
            mat3x4 = q.CandidateObjectToWorld3x4();
            mat4x3 = q.CandidateObjectToWorld4x3();
// CHECK:  OpRayQueryConfirmIntersectionKHR [[rayquery]]
            q.CommitNonOpaqueTriangleHit();
// CHECK:  OpRayQueryGetIntersectionFrontFaceKHR %bool [[rayquery]] %uint_0
            if(q.CandidateTriangleFrontFace())
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionBarycentricsKHR %v2float [[rayquery]] %uint_0
            if(q.CandidateTriangleBarycentrics().x == 0)
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionGeometryIndexKHR %uint [[rayquery]] %uint_0
            if(q.CandidateGeometryIndex())
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionInstanceCustomIndexKHR %uint [[rayquery]] %uint_0
            if(q.CandidateInstanceID())
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionInstanceIdKHR %uint [[rayquery]] %uint_0
            if(q.CandidateInstanceIndex())
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionObjectRayDirectionKHR %v3float [[rayquery]] %uint_0
            if(q.CandidateObjectRayDirection().x)
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionObjectRayOriginKHR %v3float [[rayquery]] %uint_0
            if(q.CandidateObjectRayOrigin().y)
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionPrimitiveIndexKHR %uint [[rayquery]] %uint_0
            if(q.CandidatePrimitiveIndex())
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionTKHR %float [[rayquery]] %uint_0
            if(q.CandidateTriangleRayT())
            {
                DoSomething();
            }
// CHECK:  OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR %uint [[rayquery]] %uint_0
            if(q.CandidateInstanceContributionToHitGroupIndex())
            {
                DoSomething();
            }
            break;
        case CANDIDATE_PROCEDURAL_PRIMITIVE:
        {
// CHECK:  OpRayQueryGetIntersectionWorldToObjectKHR %mat4v3float [[rayquery]] %uint_0
            mat3x4 = q.CandidateWorldToObject3x4();
            mat4x3 = q.CandidateWorldToObject4x3();
// CHECK:  OpRayQueryGetIntersectionCandidateAABBOpaqueKHR %bool
            if(q.CandidateProceduralPrimitiveNonOpaque())
            {
                DoSomething();
            }
            float t = 0.5;
// CHECK:  OpRayQueryGenerateIntersectionKHR [[rayquery]] %float_0_5
            q.CommitProceduralPrimitiveHit(t);
            q.Abort();
            break;
        }
        }
    }
    if(mat3x4[0][0] == mat4x3[0][0])
    {
        DoSomething();
    }
// CHECK:  OpRayQueryGetIntersectionTypeKHR %uint [[rayquery]] %uint_1
    switch(q.CommittedStatus())
    {
    case COMMITTED_NOTHING:
// CHECK:  OpRayQueryGetIntersectionObjectToWorldKHR %mat4v3float [[rayquery]] %uint_1
        mat3x4 = q.CommittedObjectToWorld3x4();
        mat4x3 = q.CommittedObjectToWorld4x3();
        break;
    case COMMITTED_TRIANGLE_HIT:
// CHECK:  OpRayQueryGetIntersectionWorldToObjectKHR %mat4v3float [[rayquery]] %uint_1
        mat3x4 = q.CommittedWorldToObject3x4();
        mat4x3 = q.CommittedWorldToObject4x3();
// CHECK:  OpRayQueryGetIntersectionFrontFaceKHR %bool [[rayquery]] %uint_1
        if(q.CommittedTriangleFrontFace())
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionBarycentricsKHR %v2float [[rayquery]] %uint_1
        if(q.CommittedTriangleBarycentrics().y == 0)
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionInstanceShaderBindingTableRecordOffsetKHR %uint [[rayquery]] %uint_1
        if(q.CommittedInstanceContributionToHitGroupIndex())
        {
            DoSomething();
        }
        break;
    case COMMITTED_PROCEDURAL_PRIMITIVE_HIT:
// CHECK:  OpRayQueryGetIntersectionGeometryIndexKHR %uint [[rayquery]] %uint_1
        if(q.CommittedGeometryIndex())
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionInstanceCustomIndexKHR %uint [[rayquery]] %uint_1
        if(q.CommittedInstanceID())
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionInstanceIdKHR %uint [[rayquery]] %uint_1
        if(q.CommittedInstanceIndex())
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionObjectRayDirectionKHR %v3float [[rayquery]] %uint_1
        if(q.CommittedObjectRayDirection().z)
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionObjectRayOriginKHR %v3float [[rayquery]] %uint_1
        if(q.CommittedObjectRayOrigin().x)
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionPrimitiveIndexKHR %uint [[rayquery]] %uint_1
        if(q.CommittedPrimitiveIndex())
        {
            DoSomething();
        }
// CHECK:  OpRayQueryGetIntersectionTKHR %float [[rayquery]] %uint_1
        if(q.CommittedRayT())
        {
            DoSomething();
        }
        break;
    }
    if(mat3x4[0][0] == mat4x3[0][0])
    {
        DoSomething();
    }
// CHECK:  OpRayQueryGetRayFlagsKHR %uint [[rayquery]]
    if(q.RayFlags())
    {
        DoSomething();
    }
// CHECK:  OpRayQueryGetRayTMinKHR %float [[rayquery]]
    if(q.RayTMin())
    {
        DoSomething();
    }
// CHECK:  OpRayQueryGetWorldRayDirectionKHR %v3float [[rayquery]]
    float3 o = q.WorldRayDirection();
// CHECK:  OpRayQueryGetWorldRayOriginKHR %v3float [[rayquery]]
    float3 d = q.WorldRayOrigin();
    if(o.x == d.z)
    {
        DoSomething();
    }
}
