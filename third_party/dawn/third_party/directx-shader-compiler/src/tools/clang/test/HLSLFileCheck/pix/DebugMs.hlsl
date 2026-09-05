// RUN: %dxc -Emain -Tms_6_5 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=10,parameter1=20,parameter2=30 | %FileCheck %s

// Check that the MS thread IDs are added properly

// Check we added the UAV:                                                                      v----metadata position: not important for this check
// CHECK: %PIX_DebugUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 [[S:[0-9]+]], i32 0, i1 false)
// CHECK: %ThreadIdX = call i32 @dx.op.threadId.i32(i32 93, i32 0)
// CHECK: %ThreadIdY = call i32 @dx.op.threadId.i32(i32 93, i32 1)
// CHECK: %ThreadIdZ = call i32 @dx.op.threadId.i32(i32 93, i32 2)
// CHECK: %CompareToThreadIdX = icmp eq i32 %ThreadIdX, 10
// CHECK: %CompareToThreadIdY = icmp eq i32 %ThreadIdY, 20
// CHECK: %CompareToThreadIdZ = icmp eq i32 %ThreadIdZ, 30
// CHECK: %CompareXAndY = and i1 %CompareToThreadIdX, %CompareToThreadIdY
// CHECK: %CompareAll = and i1 %CompareXAndY, %CompareToThreadIdZ

struct smallPayload {
  uint dummy;
};

struct PSInput {
  float4 position : SV_POSITION;
  float4 color : COLOR;
};

[outputtopology("triangle")]
[numthreads(3, 1, 1)] 
void main(
    in payload smallPayload small,
    in uint tid : SV_DispatchThreadID,
    in uint tig : SV_GroupIndex,
    in uint groupId : SV_GroupID,
    out vertices PSInput verts[3],
    out indices uint3 triangles[1]) {

  SetMeshOutputCounts(3 /*verts*/, 1 /*prims*/);
  verts[tid].position = float4(0, 0, 0, 0);
  verts[tid].color = float4(0,0,0,0);
  triangles[0] = uint3(0, 1, 2);
}
