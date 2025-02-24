// RUN: %dxc -T vs_6_5 -Od -E main %s | %opt -S -dxil-annotate-with-virtual-regs -hlsl-dxil-debug-instrumentation | FileCheck %s
 
// CHECK: [[RAYQUERY0:%.*]] = call i32 @dx.op.allocateRayQuery
// CHECK: [[RAYQUERY1:%.*]] = call i32 @dx.op.allocateRayQuery

// CHECK: [[PHIQUERY:%.*]] = phi i32 [ [[RAYQUERY0:%.*]], {{.*}} ], [ [[RAYQUERY0:%.*]], {{.*}} ]

// The debug instrumentation should NOT try to store this phi value: it's not an i32! (It's an opaque handle).
// CHECK-NOT: @dx.op.bufferStore{{.*}}[[PHIQUERY]]

RaytracingAccelerationStructure RTAS;
RWStructuredBuffer<int> UAV : register(u0);

float main(RayDesc rayDesc
           : RAYDESC) : OUT {
  RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery0;
  RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> rayQuery1;
  rayQuery0.TraceRayInline(RTAS, 1, 2, rayDesc);
  rayQuery1.TraceRayInline(RTAS, 1, 2, rayDesc);
  RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES> usedQuery = rayQuery0;
  if (UAV[0] == 1)
    usedQuery = rayQuery1;
  UAV[1] = usedQuery.CandidatePrimitiveIndex();
  rayQuery0.CommitNonOpaqueTriangleHit();
  if (rayQuery0.CommittedStatus() == COMMITTED_TRIANGLE_HIT) {
    UAV[4] = 42;
  }

  return 0;
}
