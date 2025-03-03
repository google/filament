// RUN: %dxc -T vs_6_5 -E main %s | FileCheck %s

// CHECK: %[[RTAS:[^ ]+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
// CHECK: %[[RQ2:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
// CHECK: %[[RQ1:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)

// Additional allocations should have been cleaned up
// CHECK-NOT: call i32 @dx.op.allocateRayQuery(i32 178,

// CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ1]], %dx.types.Handle %[[RTAS]], i32 0, i32 1,
// CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ1]], %dx.types.Handle %[[RTAS]], i32 1, i32 2,
// CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ2]], %dx.types.Handle %[[RTAS]], i32 0, i32 1,

RaytracingAccelerationStructure RTAS;

void DoTrace(RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery, RayDesc rayDesc) {
  rayQuery.TraceRayInline(RTAS, 0, 1, rayDesc);
}

int C;

float main(RayDesc rayDesc : RAYDESC) : OUT {
  RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery[2];
  DoTrace(rayQuery[1], rayDesc);
  rayQuery[1].TraceRayInline(RTAS, 1, 2, rayDesc);
  DoTrace(rayQuery[0], rayDesc);
  return 0;
}
