// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 %s | FileCheck %s
// RUN: %dxc -T lib_6_10 %s -ast-dump-implicit | FileCheck %s --check-prefix AST
// RUN: %dxc -T lib_6_10 %s -fcgl | FileCheck %s --check-prefix FCGL

// AST: `-CXXMethodDecl {{.*}} <<invalid sloc>> <invalid sloc> used TriangleObjectPositions 'BuiltInTrianglePositions &()' extern
// AST-NEXT:   |-TemplateArgument type 'BuiltInTrianglePositions'
// AST-NEXT:   |-HLSLIntrinsicAttr {{.*}} <<invalid sloc>> Implicit "op" "" 400
// AST-NEXT:   |-ConstAttr {{.*}} <<invalid sloc>> Implicit
// AST-NEXT:   `-AvailabilityAttr {{.*}} <<invalid sloc>> Implicit  6.10 0 0 ""

// AST: `-CXXMethodDecl {{.*}} <<invalid sloc>> <invalid sloc> used CandidateTriangleObjectPositions 'BuiltInTrianglePositions &()' extern
// AST:   |-TemplateArgument type 'BuiltInTrianglePositions'
// AST:   |-HLSLIntrinsicAttr {{.*}} <<invalid sloc>> Implicit "op" "" 398
// AST:   |-PureAttr {{.*}} <<invalid sloc>> Implicit
// AST:   `-AvailabilityAttr {{.*}} <<invalid sloc>> Implicit  6.10 0 0 ""

// AST `-CXXMethodDecl {{.*}} <<invalid sloc>> <invalid sloc> used CommittedTriangleObjectPositions 'BuiltInTrianglePositions &()' extern
// AST   |-TemplateArgument type 'BuiltInTrianglePositions'
// AST   |-HLSLIntrinsicAttr {{.*}} <<invalid sloc>> Implicit "op" "" 399
// AST   |-PureAttr {{.*}} <<invalid sloc>> Implicit
// AST   `-AvailabilityAttr {{.*}} <<invalid sloc>> Implicit  6.10 0 0 ""

// AST: -FunctionDecl {{.*}} <<invalid sloc>> <invalid sloc> implicit used TriangleObjectPositions 'BuiltInTrianglePositions ()' extern
// AST:  |-HLSLIntrinsicAttr {{.*}} <<invalid sloc>> Implicit "op" "" 397
// AST:  |-ConstAttr {{.*}} <<invalid sloc>> Implicit
// AST:  |-AvailabilityAttr {{.*}} <<invalid sloc>> Implicit  6.10 0 0 ""
// AST:  `-HLSLBuiltinCallAttr {{.*}} <<invalid sloc>> Implicit

// Test TriangleObjectPositions intrinsics for SM 6.10

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
RWByteAddressBuffer Output : register(u1);

struct [raypayload] Payload {
    float4 color : write(caller, closesthit, miss) : read(caller);
};

// Global TriangleObjectPositions - closesthit
// CHECK-LABEL: define void {{.*}}ClosestHit
// CHECK:   %{{.*}} = call <9 x float> @dx.op.triangleObjectPosition.f32(i32 -2147483641) ; TriangleObjectPosition()
  
// FCGL-LABEL: define void {{.*}}ClosestHit
// FCGL: call void @"dx.hl.op..void (i32, %struct.BuiltInTrianglePositions*)"(i32 397, %struct.BuiltInTrianglePositions* %{{.*}})
[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr) {
    BuiltInTrianglePositions positions = TriangleObjectPositions();
    payload.color = float4(positions.p0.x, positions.p1.y, positions.p2.z, 1.0);
}

// Global TriangleObjectPositions - anyhit
// CHECK-LABEL: define void {{.*}}AnyHit
// CHECK:   %{{.*}} = call <9 x float> @dx.op.triangleObjectPosition.f32(i32 -2147483641) ; TriangleObjectPosition()

// FCGL-LABEL: define void {{.*}}AnyHit
// FCGL: call void @"dx.hl.op..void (i32, %struct.BuiltInTrianglePositions*)"(i32 397, %struct.BuiltInTrianglePositions* %{{.*}})
[shader("anyhit")]
void AnyHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr) {
    BuiltInTrianglePositions positions = TriangleObjectPositions();
    if (positions.p0.x > 0.5)
        IgnoreHit();
}

// RayQuery CandidateTriangleObjectPositions and CommittedTriangleObjectPositions
// CHECK-LABEL: define void {{.*}}RayQueryTest
// CHECK: %{{.*}} = call <9 x float> @dx.op.rayQuery_CandidateTriangleObjectPosition.f32(i32 -2147483640, i32 %{{.*}})  ; RayQuery_CandidateTriangleObjectPosition(rayQueryHandle)
// CHECK: %{{.*}} = call <9 x float> @dx.op.rayQuery_CommittedTriangleObjectPosition.f32(i32 -2147483639, i32 %{{.*}})  ; RayQuery_CommittedTriangleObjectPosition(rayQueryHandle)

// FCGL-LABEL: define void {{.*}}RayQueryTest
// FCGL: %{{.*}} = call %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, %{{.*}}"(i32 398,
// FCGL: %{{.*}} = call %struct.BuiltInTrianglePositions* @"dx.hl.op.ro.%struct.BuiltInTrianglePositions* (i32, %{{.*}}"(i32 399,
[shader("compute")]
[numthreads(1, 1, 1)]
void RayQueryTest() {
    RayQuery<RAY_FLAG_FORCE_OPAQUE> q;
    
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(1, 0, 0);
    ray.TMin = 0.0f;
    ray.TMax = 1000.0f;
    
    q.TraceRayInline(Scene, RAY_FLAG_NONE, 0xFF, ray);
    
    while (q.Proceed()) {
        if (q.CandidateType() == CANDIDATE_NON_OPAQUE_TRIANGLE) {
            BuiltInTrianglePositions candidatePos = q.CandidateTriangleObjectPositions();
            Output.Store(0, asuint(candidatePos.p0.x));
        }
    }
    
    if (q.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
        BuiltInTrianglePositions committedPos = q.CommittedTriangleObjectPositions();
        Output.Store(4, asuint(committedPos.p1.y));
    }
}

// HitObject TriangleObjectPositions
// CHECK-LABEL: define void {{.*}}HitObjectTest
// CHECK: %{{.*}} = call <9 x float> @dx.op.hitObject_TriangleObjectPosition.f32(i32 -2147483638, %dx.types.HitObject %{{.*}})  ; HitObject_TriangleObjectPosition(hitObject)

// FCGL-LABEL: define void {{.*}}HitObjectTest
// FCGL: %{{.*}} = call %struct.BuiltInTrianglePositions* @"dx.hl.op.rn.%struct.BuiltInTrianglePositions* (i32, %dx.types.HitObject*)"(i32 400, %dx.types.HitObject* %{{.*}})
[shader("raygeneration")]
void HitObjectTest() {
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.001;
    ray.TMax = 10000.0;

    Payload payload = { float4(0, 0, 0, 0) };

    dx::HitObject hit = dx::HitObject::TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    if (hit.IsHit()) {
        BuiltInTrianglePositions positions = hit.TriangleObjectPositions();
        payload.color = float4(positions.p0 + positions.p1 + positions.p2, 1.0);
    }

    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}
