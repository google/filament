// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// OutputComplete() is called with NodeOutput
// ==================================================================

struct OUTPUT_RECORD
{
  uint value;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1024,1,1)]
void node030_outputcomplete_nodeoutput(NodeOutput<OUTPUT_RECORD> output)
{
  ThreadNodeOutputRecords<OUTPUT_RECORD> outputrecords = output.GetThreadNodeOutputRecords(1);
    // ...
  outputrecords.OutputComplete();
}

// CHECK: define void @node030_outputcomplete_nodeoutput()
// CHECK:   [[OHANDLE1:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)
// CHECK:   [[OHANDLE:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OHANDLE1]], %dx.types.NodeInfo { i32 6, i32 4 })
// CHECK:   [[RHANDLE1:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OHANDLE]], i32 1, i1 true)
// CHECK:   [[RHANDLE:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[RHANDLE1]], %dx.types.NodeRecordInfo { i32 38, i32 4 })
// CHECK:   call void @dx.op.outputComplete(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[RHANDLE]])
// CHECK:   ret void
// CHECK: }

// Metadata for node
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node030_outputcomplete_nodeoutput, !"node030_outputcomplete_nodeoutput", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// ...
// Arg #n: NodeOutput Tag (21)
// Arg #n+1: NodeOutput (metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{i32 8, i32 15, i32 13, i32 1,
// CHECK-SAME: i32 21, [[NODEOUT:![0-9]+]]
// CHECK-SAME: }

// NodeOutputs
// Arg #1: NodeIOKind Tag (1)
// Arg #2: RWOutputRecord (6)
// Arg #3: NodeRecordType Tag (2)
// Arg #4: OUTPUT_RECORD (output metadata)
// Arg #5: NodeOutputID (0)
// Arg #6: NodeID (metadata)
// ------------------------------------------------------------------
// CHECK: [[NODEOUT]] = !{[[OUTPUT:![0-9]+]]}
// CHECK: [[OUTPUT]] = !{i32 1, i32 6, i32 2, [[OUTPUT_RECORD:![0-9]+]], i32 3, i32 0, i32 0, [[NODEID:![0-9]+]]

// Metadata for output record struct
// Arg #1: Size Tag (0)
// Arg #4: 4 bytes
// ------------------------------------------------------------------
// CHECK: [[OUTPUT_RECORD:![0-9]+]] = !{i32 0, i32 4, i32 2, i32 4}

// NodeID
// ------------------------------------------------------------------
// CHECK: [[NODEID]] = !{!"output", i32 0}
