// REQUIRES: dxil-1-10
// RUN: not %dxc -T lib_6_10 %s 2>&1 | FileCheck %s

// Test that ClusterID() intrinsic is only valid in anyhit and closesthit shader stages
// ClusterID() should only be available in: anyhit, closesthit
// It should NOT be available in: compute, raygeneration, intersection, miss, callable

RWBuffer<uint> output : register(u0);

// CHECK-DAG: error{{.*}}: Opcode ClusterID not valid in shader model lib_6_10(cs).
[shader("compute")]
[numthreads(1,1,1)]
void test_compute() {
  uint cid = ClusterID();
  output[0] = cid;
}

// CHECK-DAG: error{{.*}}: Opcode ClusterID not valid in shader model lib_6_10(raygeneration).
[shader("raygeneration")]
void test_raygeneration() {
  uint cid = ClusterID();
  output[0] = cid;
}

// CHECK-DAG: error{{.*}}: Opcode ClusterID not valid in shader model lib_6_10(intersection).
[shader("intersection")]
void test_intersection() {
  uint cid = ClusterID();
  output[0] = cid;
}

struct [raypayload] MissPayload { uint dummy : write(miss) : read(caller); };

// CHECK-DAG: error{{.*}}: Opcode ClusterID not valid in shader model lib_6_10(miss).
[shader("miss")]
void test_miss(inout MissPayload payload) {
  uint cid = ClusterID();
  payload.dummy = cid;
}

struct CallableParams { uint dummy; };

// CHECK-DAG: error{{.*}}: Opcode ClusterID not valid in shader model lib_6_10(callable).
[shader("callable")]
void test_callable(inout CallableParams params) {
  uint cid = ClusterID();
  params.dummy = cid;
}

struct [raypayload] HitPayload { uint color : write(closesthit,anyhit) : read(caller); };

// Should succeed: ClusterID() in closesthit shader
[shader("closesthit")]
void test_closesthit(inout HitPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
  uint cid = ClusterID();
  payload.color = cid;
}

// Should succeed: ClusterID() in anyhit shader
[shader("anyhit")]
void test_anyhit(inout HitPayload payload, in BuiltInTriangleIntersectionAttributes attr) {
  uint cid = ClusterID();
  payload.color = cid;
}

