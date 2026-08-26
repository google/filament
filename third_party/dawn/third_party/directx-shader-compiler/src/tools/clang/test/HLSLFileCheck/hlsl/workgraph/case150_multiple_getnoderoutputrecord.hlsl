// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Multiple calls to getnodeouputrecord(array)
// ==================================================================

struct RECORD {
  int i;
  float3 foo;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1024, 1, 1)]
[NodeDispatchGrid(16, 1, 1)]
void node150_a(NodeOutput<RECORD> output)
{
  GroupNodeOutputRecords<RECORD> outRec1 = output.GetGroupNodeOutputRecords(1);
  GroupNodeOutputRecords<RECORD> outRec2 = output.GetGroupNodeOutputRecords(4);
  outRec1.OutputComplete();
  outRec2.OutputComplete();
}
// CHECK: define void @node150_a() {
// CHECK: [[OP_HANDLE:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK: [[ANN_OP_HANDLE:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OP_HANDLE]], %dx.types.NodeInfo { i32 6, i32 16 })
// CHECK: [[OP_REC_HANDLE1:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 238, %dx.types.NodeHandle [[ANN_OP_HANDLE]], i32 1, i1 false)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK: [[ANN_OP_REC_HANDLE1:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[OP_REC_HANDLE1]], %dx.types.NodeRecordInfo { i32 70, i32 16 })
// CHECK: [[OP_REC_HANDLE2:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 238, %dx.types.NodeHandle [[ANN_OP_HANDLE]], i32 4, i1 false)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK: [[ANN_OP_REC_HANDLE2:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[OP_REC_HANDLE2]], %dx.types.NodeRecordInfo { i32 70, i32 16 })
// CHECK: call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle [[ANN_OP_REC_HANDLE1]])  ; OutputComplete(output)
// CHECK: call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle [[ANN_OP_REC_HANDLE2]])  ; OutputComplete(output)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1024, 1, 1)]
[NodeDispatchGrid(16, 1, 1)]
void node150_b(NodeOutput<RECORD> output)
{
  ThreadNodeOutputRecords<RECORD> outRec1 = output.GetThreadNodeOutputRecords(5);
  ThreadNodeOutputRecords<RECORD> outRec2 = output.GetThreadNodeOutputRecords(1);
  outRec1.OutputComplete();
  outRec1 = outRec2;
  outRec1.OutputComplete();

}
// CHECK: define void @node150_b() {
// CHECK: [[OP_HANDLE:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK: [[ANN_OP_HANDLE:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OP_HANDLE]], %dx.types.NodeInfo { i32 6, i32 16 })
// CHECK: [[OP_REC_HANDLE1:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_OP_HANDLE]], i32 5, i1 true)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK: [[ANN_OP_REC_HANDLE1:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[OP_REC_HANDLE1]], %dx.types.NodeRecordInfo { i32 38, i32 16 })
// CHECK: [[OP_REC_HANDLE2:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_OP_HANDLE]], i32 1, i1 true)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK: [[ANN_OP_REC_HANDLE2:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[OP_REC_HANDLE2]], %dx.types.NodeRecordInfo { i32 38, i32 16 })
// CHECK: call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle [[ANN_OP_REC_HANDLE1]])  ; OutputComplete(output)
// CHECK: call void @dx.op.outputComplete(i32 241, %dx.types.NodeRecordHandle [[ANN_OP_REC_HANDLE2]])  ; OutputComplete(output)
