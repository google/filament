// RUN: %dxc -T vs_6_5 -E main %s | FileCheck %s

RaytracingAccelerationStructure RTAS;

// We should eliminate these calls somehow in the future, but for now, that does not look like a legal optimization.
// CHECK: call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
// CHECK: call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
// CHECK: call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
// CHECK: call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
static RayQuery<0> g_rayQueryArray[4];

// g_rayQueryUnused should be optimized away
static RayQuery<0> g_rayQueryUnused;

void main(uint i : IDX, RayDesc rayDesc : RAYDESC) {
  // CHECK: %[[rayQuery0a:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
  RayQuery<0> rayQuery0a;

  // rayQuery0b should be completely optimized away
  // CHECK-NOT: call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
  RayQuery<0> rayQuery0b;
  g_rayQueryArray[i] = rayQuery0b;  // Stored here, then overwritten with rayQuery0a
  g_rayQueryArray[i] = rayQuery0a;

  // No separate allocation, just a handle copy
  // optimizations should have eliminated load from global array
  // CHECK-NOT: load
  RayQuery<0> rayQuery0c = g_rayQueryArray[i];

  // rayQuery0a is the one actually used here
  // CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[rayQuery0a]],
  rayQuery0c.TraceRayInline(RTAS, 1, 2, rayDesc);

  // AllocateRayQuery occurs here, rather than next to allocas
  // Should not be extray allocate, since above should allocate and copy
  // CHECK: %[[rayQuery1c:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 1)
  // CHECK-NOT: call i32 @dx.op.allocateRayQuery(i32 178, i32 0)
  RayQuery<RAY_FLAG_FORCE_OPAQUE> rayQuery1c = RayQuery<RAY_FLAG_FORCE_OPAQUE>();

  // CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[rayQuery1c]],
  rayQuery1c.TraceRayInline(RTAS, 3, 4, rayDesc);
}
