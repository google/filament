// RUN: %dxc -T vs_6_5 -E main %s | FileCheck %s

// CHECK: %[[RTAS:[^ ]+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
RaytracingAccelerationStructure RTAS;

void DoTrace(RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery, RayDesc rayDesc) {
  rayQuery.TraceRayInline(RTAS, 0, 1, rayDesc);
}

int C;

float main(RayDesc rayDesc : RAYDESC) : OUT {
  // CHECK: %[[array:[^ ]+]] = alloca [6 x i32]
  // Ideally, one for [1][2] statically indexed, and 3 for [0][C] dynamically indexed sub-array.
  // But that would require 2d array optimization when one index is constant.
  // CHECK: %[[RQ00:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  // CHECK: %[[RQ01:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  // CHECK: %[[RQ02:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  // CHECK: %[[RQ10:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  // CHECK: %[[RQ11:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  // CHECK: %[[RQ12:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
  RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery[2][3];

  // CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ12]], %dx.types.Handle %[[RTAS]], i32 0, i32 1,
  DoTrace(rayQuery[1][2], rayDesc);
  // CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ12]], %dx.types.Handle %[[RTAS]], i32 1, i32 2,
  rayQuery[1][2].TraceRayInline(RTAS, 1, 2, rayDesc);

  // CHECK: %[[GEP:[^ ]+]] = getelementptr [6 x i32], [6 x i32]* %[[array]],
  // CHECK: %[[load:[^ ]+]] = load i32, i32* %[[GEP]]
  // CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[load]], %dx.types.Handle %[[RTAS]], i32 0, i32 1,
  DoTrace(rayQuery[0][C], rayDesc);
  return 0;
}
