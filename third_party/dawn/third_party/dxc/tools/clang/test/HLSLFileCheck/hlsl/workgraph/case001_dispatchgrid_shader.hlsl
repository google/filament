// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// Broadcasting launch node with dispatch grid defined in shader
// ==================================================================

struct INPUT_NOGRID
{
  uint textureIndex;
};

// Shader function
// Arg #1: Opcode = <CreateInputRecordHandle>
// Arg #2: Metadata ID
// ------------------------------------------------------------------
// CHECK: define void @node001_dispatchgrid_shader()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2,3,2)]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node001_dispatchgrid_shader(DispatchNodeInputRecord<INPUT_NOGRID> input)
{
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node001_dispatchgrid_shader, !"node001_dispatchgrid_shader", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// Arg #5: NodeIsProgramEntry Tag (14)
// Arg #6: True (1)
// Arg #7: NodeId Tag (15)
// Arg #8: NodeId (NodeId metadata)
// Arg #9: NodeLocalRootArgumentsTableIndex Tag (16)
// Arg #10: Index (-1)
// Arg #11: NodeDispatchGrid Tag (18)
// Arg #12: NodeDispatchGrid (xyz metadata)
// Arg #13: NodeInputs Tag (20)
// Arg #14: NodeInputs (NodeInput metadata)
// Arg #15: NumThreads Tag (4)
// Arg #16: NumThreads (xyz metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 1, i32 14, i1 true, i32 15, [[NODEID:![0-9]+]], i32 16, i32 -1, i32 18, [[DISPATCHGRID:![0-9]+]], i32 20, [[NODE_IN:![0-9]+]], i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK-SAME: }

// DispatchGrid
// Arg #1: 2
// Arg #2: 3
// Arg #3: 2
// ------------------------------------------------------------------
// CHECK: [[DISPATCHGRID]] = !{i32 2, i32 3, i32 2}

// NodeInputs
// Arg #1: NodeIOFlags Tag (1)
// Arg #2: DispatchNodeInputRecord (97)
// Arg #3: NodeRecordType Tag (2)
// Arg #4: INPUT_NOGRID type
// ------------------------------------------------------------------
// CHECK: [[NODE_IN]] = !{[[INPUT0:![0-9]+]]}
// CHECK: [[INPUT0]] = !{i32 1, i32 97, i32 2, [[INPUT_NOGRID:![0-9]+]]}

// Metadata for input record struct
// Arg #1: Size Tag (0)
// Arg #2: 4 bytes
// ------------------------------------------------------------------
// CHECK-DAG: [[INPUT_NOGRID]] = !{i32 0, i32 4, i32 2, i32 4}

// NumThreads
// Arg #1: 1024
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK: [[NUMTHREADS]] = !{i32 1024, i32 1, i32 1}
