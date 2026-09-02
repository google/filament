// RUN: %dxc -T lib_6_8 external %s | FileCheck %s
// ==================================================================
// GetInputRecordCount() called with NodeInputRecordArray
// ==================================================================

RWBuffer<uint> buf0;

struct INPUT_RECORD
{
    uint textureIndex;
};

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node014_getrecordcount([MaxRecords(256)] GroupNodeInputRecords<INPUT_RECORD> inputs)
{
  uint numRecords = inputs.Count();
  buf0[0] = numRecords;
}

// Shader function
// Arg #1: Opcode = <CreateNodeInputHandle>
// Arg #2: Metadata ID
// ------------------------------------------------------------------
// CHECK: define void @node014_getrecordcount()
// CHECK-SAME: {
// CHECK:  [[INPUTS:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)
// CHECK:  [[ANN_IP:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[INPUTS]], %dx.types.NodeRecordInfo { i32 65, i32 4 }) 
// CHECK:  {{%[0-9]+}} = call i32  @dx.op.getInputRecordCount(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_IP]])
// CHECK:   ret void
// CHECK: }

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node014_getrecordcount, !"node014_getrecordcount", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: Shader Kind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: coalescing (2)
// ...
// Arg #n: NodeInputs Tag (20)
// Arg #n+1: NodeInputs (metadata)
// ...
// Arg #m: NumThreads Tag (4)
// Arg #m+1: NumThreads (xyz metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 2,
// CHECK-SAME: i32 20, [[NODE_IN:![0-9]+]]
// CHECK-SAME: i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK-SAME: }

// NodeInputs
// Arg #1: NodeIOKind Tag (1)
// Arg #2: InputRecord (65)
// Arg #3: NodeRecordType Tag Tag (2)
// Arg #4: 256
// Arg #5: NodeMaxRecords Tag Tag (3)
// Arg #6: INPUT_RECORD Type
// ------------------------------------------------------------------
// CHECK-DAG: [[NODE_IN]] = !{[[INPUT0:![0-9]+]]}
// CHECK-DAG: [[INPUT0]] = !{i32 1, i32 65, i32 2, [[INPUT_RECORD:![0-9]+]], i32 3, i32 256}


// Metadata for input record struct
// Arg #1: Size Tag (0)
// Arg #2: 4 bytes
// ------------------------------------------------------------------
// CHECK: [[INPUT_RECORD:![0-9]+]] = !{i32 0, i32 4, i32 2, i32 4}

// NumThreads
// Arg #1: 1024
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK-DAG: [[NUMTHREADS]] = !{i32 1024, i32 1, i32 1}
