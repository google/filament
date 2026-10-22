// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s
// ==================================================================
// Thread launch node declares EmptyNodeInput<1>
// ==================================================================

RWBuffer<uint> buf0;

[Shader("node")]
[NodeLaunch("coalescing")]
[NodeIsProgramEntry]
[NumThreads(2,1,1)]
void node085_thread_emptynodeinput(EmptyNodeInput input)
{
  // input.Count should always return 1 here, so there is
  // an opportunity for an optimization.
  buf0[0] = input.Count();
}

// CHECK: define void @node085_thread_emptynodeinput() {
// CHECK: [[LOAD:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?buf0@@3V?$RWBuffer@I@@A"
// CHECK: [[HANDLE:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_HANDLE:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[HANDLE]], %dx.types.NodeRecordInfo { i32 9, i32 0 })
// CHECK: [[COUNT:%[0-9]+]] = call i32 @dx.op.getInputRecordCount(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_HANDLE]])  ; GetInputRecordCount(input)
// CHECK: [[HANDLE_FOR_LIB:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle [[LOAD]])  ; CreateHandleForLib(Resource)
// CHECK: [[ANN_HANDLE2:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle [[HANDLE_FOR_LIB]], %dx.types.ResourceProperties { i32 4106, i32 261 })  ; AnnotateHandle(res,props)  resource: RWTypedBuffer<U32>
// CHECK: call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle [[ANN_HANDLE2]], i32 0, i32 undef, i32 [[COUNT]], i32 [[COUNT]], i32 [[COUNT]], i32 [[COUNT]], i8 15) ; BufferStore(uav,coord0,coord1,value0,value1,value2,value3,mask)

// Metadata for node
// ------------------------------------------------------------------
// CHECK-DAG: = !{void ()* @node085_thread_emptynodeinput, !"node085_thread_emptynodeinput", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #2: coalescing (2)
// ...
// Arg #n: NodeInputs Tag (20)
// Arg #n+1: NodeInputs (metadata)
// ------------------------------------------------------------------
// CHECK-DAG: [[ATTRS]] = !{{{.*}}i32 8, i32 15, i32 13, i32 2,{{.*}}i32 20, [[NODE_IN:![0-9]+]]{{.*}}}

// NodeInputs
// Arg #1: NodeIOKind Tag (1)
// Arg #2: EmptyNodeInput (9)
// Arg #3: NodeInputMaxRecordArraySize Tag (2)
// Arg #4: MaxRecordArraySize = 1
// ------------------------------------------------------------------
// CHECK-DAG: [[NODE_IN]] = !{[[INPUT0:![0-9]+]]}
// CHECK-DAG: [[INPUT0]] = !{i32 1, i32 9}
