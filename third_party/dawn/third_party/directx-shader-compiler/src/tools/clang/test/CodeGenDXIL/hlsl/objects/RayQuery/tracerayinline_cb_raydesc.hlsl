// RUN: %dxc -T vs_6_5 -E main %s | FileCheck %s

// CHECK-DAG: %[[RTAS:[^ ]+]] = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)
// CHECK-DAG: %[[RQ:[^ ]+]] = call i32 @dx.op.allocateRayQuery(i32 178, i32 513)
// CHECK: call void @dx.op.rayQuery_TraceRayInline(i32 179, i32 %[[RQ]], %dx.types.Handle %[[RTAS]], i32 1, i32 2,

RaytracingAccelerationStructure RTAS;

RayDesc rayDesc;

void main() {
  RayQuery<RAY_FLAG_FORCE_OPAQUE|RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery;
  rayQuery.TraceRayInline(RTAS, 1, 2, rayDesc);
}
