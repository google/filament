// REQUIRES: dxil-1-9
// RUN: %dxc -T lib_6_9 %s | FileCheck %s 
// RUN: %dxc -T lib_6_9 -fcgl %s | FileCheck -check-prefix=FCGL %s 

// RUN: %dxc -T vs_6_9 %s | FileCheck %s 
// RUN: %dxc -T vs_6_9 -fcgl %s | FileCheck -check-prefix=FCGL %s 


RaytracingAccelerationStructure RTAS;
[shader("vertex")]
void main(RayDesc rayDesc : RAYDESC) {

  // CHECK: call i32 @dx.op.allocateRayQuery2(i32 258, i32 1024, i32 1)
  // FCGL: call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 1024, i32 1)
  RayQuery<RAY_FLAG_FORCE_OMM_2_STATE, RAYQUERY_FLAG_ALLOW_OPACITY_MICROMAPS> rayQuery1;

  rayQuery1.TraceRayInline(RTAS, RAY_FLAG_FORCE_OMM_2_STATE, 2, rayDesc);

  // CHECK: call i32 @dx.op.allocateRayQuery(i32 178, i32 1)
  // FCGL: call i32 @"dx.hl.op..i32 (i32, i32, i32)"(i32 4, i32 1, i32 0)
  RayQuery<RAY_FLAG_FORCE_OPAQUE> rayQuery2;
  rayQuery2.TraceRayInline(RTAS, 0, 2, rayDesc);
}
