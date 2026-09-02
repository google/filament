// RUN: %dxc -T lib_6_9 -verify %s

// Test that ClusterID intrinsics require SM 6.10

RWByteAddressBuffer outbuf : register(u0);

struct [raypayload] Payload {
  float dummy : read(caller, closesthit, miss, anyhit) : write(caller, closesthit, miss, anyhit);
};

// Global ClusterID intrinsic
[shader("closesthit")]
void test_cluster_id(inout Payload payload, in BuiltInTriangleIntersectionAttributes attr) {
  uint cid = ClusterID(); // expected-error {{intrinsic ClusterID potentially used by ''test_cluster_id'' requires shader model 6.10 or greater}}
  outbuf.Store(0, cid);
}

// RayQuery CandidateClusterID
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
  uint cid = rq.CandidateClusterID(); // expected-error {{intrinsic RayQuery<0, 0>::CandidateClusterID potentially used by ''test_rayquery_candidate_cluster_id'' requires shader model 6.10 or greater}}
  outbuf.Store(4, cid);
}

// RayQuery CommittedClusterID
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
  uint cid = rq.CommittedClusterID(); // expected-error {{intrinsic RayQuery<0, 0>::CommittedClusterID potentially used by ''test_rayquery_committed_cluster_id'' requires shader model 6.10 or greater}}
  outbuf.Store(8, cid);
}

// HitObject GetClusterID
[shader("raygeneration")]
void test_hitobject_cluster_id() {
  dx::HitObject ho = dx::HitObject::MakeNop();
  uint cid = ho.GetClusterID(); // expected-error {{intrinsic dx::HitObject::GetClusterID potentially used by ''test_hitobject_cluster_id'' requires shader model 6.10 or greater}}
  outbuf.Store(12, cid);
}

