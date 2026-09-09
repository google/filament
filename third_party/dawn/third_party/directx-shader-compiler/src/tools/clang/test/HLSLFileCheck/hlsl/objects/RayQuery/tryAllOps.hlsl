// RUN: %dxc -T cs_6_5 -E CS %s | FileCheck %s

// CHECK: define void @CS()

// RayQuery alloca should have been dead-code eliminated
// CHECK-NOT: alloca

// CHECK: %[[hAccelerationStructure:[^ ]+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
// CHECK: %[[hRayQuery:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 5)
// CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[hRayQuery]], %dx.types.Handle %[[hAccelerationStructure]], i32 0, i32 255, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 0.000000e+00, float 1.000000e+00, float 0.000000e+00, float 0.000000e+00, float 9.999000e+03)
// CHECK: call i1 @dx.op.rayQuery_Proceed.i1(i32 180, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 185, i32 %[[hRayQuery]])
// CHECK: call void @dx.op.rayQuery_Abort(i32 181, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateMatrix.f32(i32 186, i32 %[[hRayQuery]], i32 0, i8 0)
// CHECK: call void @dx.op.rayQuery_CommitNonOpaqueTriangleHit(i32 182, i32 %[[hRayQuery]])
// CHECK: call i1 @dx.op.rayQuery_StateScalar.i1(i32 191, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 193, i32 %[[hRayQuery]], i8 0)
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 203, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 202, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 201, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 206, i32 %[[hRayQuery]], i8 0)
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 205, i32 %[[hRayQuery]], i8 1)
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 204, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateScalar.f32(i32 199, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 214, i32 %[[hRayQuery]])
// CHECK: call i1 @dx.op.rayQuery_Proceed.i1(i32 180, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateMatrix.f32(i32 187, i32 %[[hRayQuery]], i32 0, i8 0)
// CHECK: call i1 @dx.op.rayQuery_StateScalar.i1(i32 190, i32 %[[hRayQuery]])
// CHECK: call void @dx.op.rayQuery_CommitProceduralPrimitiveHit(i32 183, i32 %[[hRayQuery]], float 5.000000e-01)
// CHECK: call void @dx.op.rayQuery_Abort(i32 181, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 184, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateMatrix.f32(i32 188, i32 %[[hRayQuery]], i32 0, i8 0)
// CHECK: call float @dx.op.rayQuery_StateMatrix.f32(i32 189, i32 %[[hRayQuery]], i32 0, i8 0)
// CHECK: call i1 @dx.op.rayQuery_StateScalar.i1(i32 192, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 194, i32 %[[hRayQuery]], i8 1)
// CHECL: call i32 @dx.op.rayQuery_StateScalar.i32(i32 215, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 209, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 208, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 207, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 212, i32 %[[hRayQuery]], i8 2)
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 211, i32 %[[hRayQuery]], i8 0)
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 210, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateScalar.f32(i32 200, i32 %[[hRayQuery]])
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 195, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateScalar.f32(i32 198, i32 %[[hRayQuery]])
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 197, i32 %[[hRayQuery]], i8 0)
// CHECK: call float @dx.op.rayQuery_StateVector.f32(i32 196, i32 %[[hRayQuery]], i8 2)

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
    while(q.Proceed())
    {
        switch(q.CandidateType())
        {
        case CANDIDATE_NON_OPAQUE_TRIANGLE:
            q.Abort();
            mat3x4 = q.CandidateObjectToWorld3x4();
            mat4x3 = q.CandidateObjectToWorld4x3();
            q.CommitNonOpaqueTriangleHit();
            if(q.CandidateTriangleFrontFace())
            {
                DoSomething();
            }
            if(q.CandidateTriangleBarycentrics().x == 0)
            {
                DoSomething();
            }
            if(q.CandidateGeometryIndex())
            {
                DoSomething();
            }
            if(q.CandidateInstanceID())
            {
                DoSomething();
            }
            if(q.CandidateInstanceIndex())
            {
                DoSomething();
            }
            if(q.CandidateObjectRayDirection().x)
            {
                DoSomething();
            }
            if(q.CandidateObjectRayOrigin().y)
            {
                DoSomething();
            }
            if(q.CandidatePrimitiveIndex())
            {
                DoSomething();
            }
            if(q.CandidateTriangleRayT())
            {
                DoSomething();
            }
            if(q.CandidateInstanceContributionToHitGroupIndex())
            {
                DoSomething();
            }
            break;
        case CANDIDATE_PROCEDURAL_PRIMITIVE:
        {
            mat3x4 = q.CandidateWorldToObject3x4();
            mat4x3 = q.CandidateWorldToObject4x3();
            if(q.CandidateProceduralPrimitiveNonOpaque())
            {
                DoSomething();
            }
            float t = 0.5;
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
    switch(q.CommittedStatus())
    {
    case COMMITTED_NOTHING:
        mat3x4 = q.CommittedObjectToWorld3x4();
        mat4x3 = q.CommittedObjectToWorld4x3();
        break;
    case COMMITTED_TRIANGLE_HIT:
        mat3x4 = q.CommittedWorldToObject3x4();
        mat4x3 = q.CommittedWorldToObject4x3();
        if(q.CommittedTriangleFrontFace())
        {
            DoSomething();
        }
        if(q.CommittedTriangleBarycentrics().y == 0)
        {
            DoSomething();
        }
        if(q.CommittedInstanceContributionToHitGroupIndex())
        {
            DoSomething();
        }
        break;
    case COMMITTED_PROCEDURAL_PRIMITIVE_HIT:
        if(q.CommittedGeometryIndex())
        {
            DoSomething();
        }
        if(q.CommittedInstanceID())
        {
            DoSomething();
        }
        if(q.CommittedInstanceIndex())
        {
            DoSomething();
        }
        if(q.CommittedObjectRayDirection().z)
        {
            DoSomething();
        }
        if(q.CommittedObjectRayOrigin().x)
        {
            DoSomething();
        }
        if(q.CommittedPrimitiveIndex())
        {
            DoSomething();
        }
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
    if(q.RayFlags())
    {
        DoSomething();
    }
    if(q.RayTMin())
    {
        DoSomething();
    }
    float3 o = q.WorldRayDirection();
    float3 d = q.WorldRayOrigin();
    if(o.x == d.z)
    {
        DoSomething();
    }
}