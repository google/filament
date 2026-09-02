// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 %s | FileCheck %s
// RUN: %dxc -T lib_6_10 %s -ast-dump-implicit | FileCheck %s --check-prefix AST
// RUN: %dxc -T lib_6_10 %s -fcgl | FileCheck %s --check-prefix FCGL

// Test ClusterID intrinsics for SM 6.10

// AST: `-CXXMethodDecl {{.*}} used GetClusterID 'unsigned int ()' extern
// AST-NEXT: {{.*}}|-TemplateArgument type 'unsigned int'
// AST-NEXT: {{.*}}|-HLSLIntrinsicAttr {{.*}} Implicit "op" "" 396
// AST-NEXT: {{.*}}|-ConstAttr {{.*}} Implicit
// AST-NEXT: {{.*}}`-AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""

// AST: `-CXXMethodDecl {{.*}} used CandidateClusterID 'unsigned int ()' extern
// AST-NEXT: {{.*}}|-TemplateArgument type 'unsigned int'
// AST-NEXT: {{.*}}|-HLSLIntrinsicAttr {{.*}} Implicit "op" "" 394
// AST-NEXT: {{.*}}|-PureAttr {{.*}} Implicit
// AST-NEXT: {{.*}}`-AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""

// AST: `-CXXMethodDecl {{.*}} used CommittedClusterID 'unsigned int ()' extern
// AST-NEXT: {{.*}}|-TemplateArgument type 'unsigned int'
// AST-NEXT: {{.*}}|-HLSLIntrinsicAttr {{.*}} Implicit "op" "" 395
// AST-NEXT: {{.*}}|-PureAttr {{.*}} Implicit
// AST-NEXT: {{.*}}`-AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""

// AST: -FunctionDecl {{.*}} implicit used ClusterID 'unsigned int ()' extern
// AST-NEXT: {{.*}}|-HLSLIntrinsicAttr {{.*}} Implicit "op" "" 393
// AST-NEXT: {{.*}}|-ConstAttr {{.*}} Implicit
// AST-NEXT: {{.*}}|-AvailabilityAttr {{.*}} Implicit  6.10 0 0 ""
// AST-NEXT: {{.*}}`-HLSLBuiltinCallAttr {{.*}} Implicit

RWByteAddressBuffer outbuf : register(u0);

struct [raypayload] Payload {
  float dummy : read(caller, closesthit, miss, anyhit) : write(caller, closesthit, miss, anyhit);
};

// Global ClusterID intrinsic
// CHECK-LABEL: define void @{{.*}}test_cluster_id{{.*}}(
// CHECK: call i32 @dx.op.clusterID(i32 -2147483645)
// CHECK: call void @dx.op.rawBufferStore.i32

// FCGL-LABEL: define void @{{.*}}test_cluster_id{{.*}}(
// FCGL: call i32 @"dx.hl.op.rn.i32 (i32)"(i32 393)
[shader("closesthit")]
void test_cluster_id(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr) {
  uint cid = ClusterID();
  outbuf.Store(0, cid);
}

// RayQuery CandidateClusterID
// CHECK-LABEL: define void @{{.*}}test_rayquery_candidate_cluster_id{{.*}}(
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483644
// CHECK: call void @dx.op.rawBufferStore.i32

// FCGL-LABEL: define void @{{.*}}test_rayquery_candidate_cluster_id{{.*}}(
// FCGL: call i32 @"dx.hl.op.ro.i32 (i32, %{{.*}}"(i32 394
[shader("raygeneration")]
void test_rayquery_candidate_cluster_id() {
  RayQuery<RAY_FLAG_NONE> rq;
  RaytracingAccelerationStructure as;
  RayDesc ray;
  ray.Origin = float3(0, 0, 0);
  ray.Direction = float3(0, 0, 1);
  ray.TMin = 0.0;
  ray.TMax = 1000.0;
  
  rq.TraceRayInline(as, 0, 0xff, ray);
  rq.Proceed();
  uint cid = rq.CandidateClusterID();
  outbuf.Store(4, cid);
}

// RayQuery CommittedClusterID
// CHECK-LABEL: define void @{{.*}}test_rayquery_committed_cluster_id{{.*}}(
// CHECK: call i32 @dx.op.rayQuery_StateScalar.i32(i32 -2147483643
// CHECK: call void @dx.op.rawBufferStore.i32

// FCGL-LABEL: define void @{{.*}}test_rayquery_committed_cluster_id{{.*}}(
// FCGL: call i32 @"dx.hl.op.ro.i32 (i32, %{{.*}}"(i32 395
[shader("raygeneration")]
void test_rayquery_committed_cluster_id() {
  RayQuery<RAY_FLAG_NONE> rq;
  RaytracingAccelerationStructure as;
  RayDesc ray;
  ray.Origin = float3(0, 0, 0);
  ray.Direction = float3(0, 0, 1);
  ray.TMin = 0.0;
  ray.TMax = 1000.0;
  
  rq.TraceRayInline(as, 0, 0xff, ray);
  uint cid = rq.CommittedClusterID();
  outbuf.Store(8, cid);
}

// HitObject GetClusterID
// CHECK-LABEL: define void @{{.*}}test_hitobject_cluster_id{{.*}}(
// CHECK: call i32 @dx.op.hitObject_StateScalar.i32(i32 -2147483642
// CHECK: call void @dx.op.rawBufferStore.i32

// FCGL-LABEL: define void @{{.*}}test_hitobject_cluster_id{{.*}}(
// FCGL: call i32 @"dx.hl.op.rn.i32 (i32, %dx.types.HitObject{{.*}}"(i32 396
[shader("raygeneration")]
void test_hitobject_cluster_id() {
  dx::HitObject ho = dx::HitObject::MakeNop();
  uint cid = ho.GetClusterID();
  outbuf.Store(12, cid);
}

