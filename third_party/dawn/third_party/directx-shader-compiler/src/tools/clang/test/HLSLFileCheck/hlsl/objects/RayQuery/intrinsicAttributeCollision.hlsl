// RUN: %dxc -T cs_6_5 -E CS -fcgl %s | FileCheck %s
// RUN: %dxc -T cs_6_5 -E CS %s | FileCheck %s -check-prefix=CHECKDXIL

// Proceed called before CommittedTriangleFrontFace.
// Don't be sensitive to HL Opcode because those can change.
// CHECK: call i1 [[HLProceed:@"[^"]+"]](i32
// CHECK: call i1 [[HLCommittedTriangleFrontFace:@"[^".]+\.[^.]+\.[^.]+\.ro[^"]+"]](i32
// ^ matches call i1 @"dx.hl.op.ro.i1 (i32, %\22class.RayQuery<5>\22*)"(i32
// CHECK-LABEL: ret void,

// Ensure HL declarations are not collapsed when attributes differ
// CHECK-DAG: declare i1 [[HLProceed]]({{.*}}) #[[AttrProceed:[0-9]+]]
// CHECK-DAG: declare i1 [[HLCommittedTriangleFrontFace]]({{.*}}) #[[AttrCommittedTriangleFrontFace:[0-9]+]]

// Ensure correct attributes for each HL intrinsic
// CHECK-DAG: attributes #[[AttrProceed]] = { nounwind }
// CHECK-DAG: attributes #[[AttrCommittedTriangleFrontFace]] = { nounwind readonly }

// Ensure Proceed not eliminated in final DXIL:
// CHECKDXIL: call i1 @dx.op.rayQuery_Proceed.i1(i32 180,
// CHECKDXIL: call i1 @dx.op.rayQuery_StateScalar.i1(i32 192,

RaytracingAccelerationStructure AccelerationStructure : register(t0);
RWByteAddressBuffer log : register(u0);

[numThreads(1,1,1)]
void CS()
{
    RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> q;
    RayDesc ray = { {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, 0.0f, 9999.0f};
    q.TraceRayInline(AccelerationStructure, RAY_FLAG_NONE, 0xFF, ray);

    q.Proceed();

    if(q.CommittedTriangleFrontFace())
    {
	    log.Store(0,1);
    }
}
