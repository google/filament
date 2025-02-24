// RUN: %dxc -Emain -Tvs_6_0 %s | %opt -S -hlsl-dxil-debug-instrumentation,parameter0=1,parameter1=2 -hlsl-dxilemit | %FileCheck %s

// Check that the instance and vertex id are parsed and present:

// Check we added the UAV:                                                                      v----metadata position: not important for this check
// CHECK: %PIX_DebugUAV_Handle = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 [[S:[0-9]+]], i32 0, i1 false)
// CHECK: %VertId = call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 0, i8 0, i32 undef)
// CHECK: %InstanceId = call i32 @dx.op.loadInput.i32(i32 4, i32 1, i32 0, i8 0, i32 undef)
// CHECK: %CompareToVertId = icmp eq i32 %VertId, 1
// CHECK: %CompareToInstanceId = icmp eq i32 %InstanceId, 2
// CHECK: %CompareBoth = and i1 %CompareToVertId, %CompareToInstanceId

// Check that the correct metadata was emitted for vertex id and instance id. 
// They should have 1 row, 1 column each. Vertex ID first at row 0, then instnce at row 1.
// (With each row have the same value as the corresponding ID)
// See DxilMDHelper::EmitSignatureElement for the meaning of these entries:
//               ID                   TypeU32 SemKin Sem-Idx         interp  Rows  Cols   Row   Col
//                |                     |     |       |                |      |     |      |     |
// CHECK: = !{i32 0, !"SV_VertexID", i8 5, i8 1, ![[VIDID:[0-9]*]], i8 1, i32 1, i8 1, i32 0, i8 0,
//                |                       |     |     |                |      |     |      |     |
// CHECK: = !{i32 1, !"SV_InstanceID", i8 5, i8 2, ![[IID:[0-9]*]], i8 1, i32 1, i8 1, i32 1, i8 0,
[RootSignature("")]
float4 main() : SV_Position{
  return float4(0,0,0,0);
}