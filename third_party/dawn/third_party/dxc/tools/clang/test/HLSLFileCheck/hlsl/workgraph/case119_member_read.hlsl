// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Read access to members of input/output records
// ==================================================================

RWBuffer<uint> buf0;

struct RECORD
{
  uint a;
  uint b;
  uint c;
};

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(16,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().a;
}

// CHECK: define void @node01() {
// CHECK: [[NODE01_L:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)
// CHECK: [[ANN_NODE01_L:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE01_L]], %dx.types.NodeRecordInfo { i32 97, i32 12 }) 
// CHECK: {{[0-9]+}} = call %struct.RECORD addrspace(6)* @dx.op.getNodeRecordPtr.struct.RECORD(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE01_L]], i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(16,1,1)]
[NodeLaunch("broadcasting")]
void node02(RWDispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().b;
}

// CHECK: define void @node02() {
// CHECK: [[NODE02_L:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)
// CHECK: [[ANN_NODE02_L:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE02_L]], %dx.types.NodeRecordInfo { i32 101, i32 12 }) 
// CHECK: {{[0-9]+}} = call %struct.RECORD addrspace(6)* @dx.op.getNodeRecordPtr.struct.RECORD(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE02_L]], i32 0)

[Shader("node")]
[NumThreads(1024, 1, 1)]
[NodeLaunch("coalescing")]
void node03([MaxRecords(3)] GroupNodeInputRecords<RECORD> input)
{
  buf0[0] = input[1].c;
}

// CHECK: define void @node03() {
// CHECK: [[NODE03_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_NODE03_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE03_A]], %dx.types.NodeRecordInfo { i32 65, i32 12 })
// CHECK: {{[0-9]+}} = call %struct.RECORD addrspace(6)* @dx.op.getNodeRecordPtr.struct.RECORD(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE03_A]], i32 1)


[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node04([MaxRecords(4)] RWGroupNodeInputRecords<RECORD> input)
{
  buf0[0] = input[2].c;  
}

// CHECK: define void @node04() {
// CHECK: [[NODE04_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_NODE04_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE04_A]], %dx.types.NodeRecordInfo { i32 69, i32 12 })
// CHECK: {{[0-9]+}} = call %struct.RECORD addrspace(6)* @dx.op.getNodeRecordPtr.struct.RECORD(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE04_A]], i32 2)

// TODO: add test cases for OutputRecord, OutputRecordArray, GroupSharedOutputRecord, and GroupSharedOutputRecordArray
