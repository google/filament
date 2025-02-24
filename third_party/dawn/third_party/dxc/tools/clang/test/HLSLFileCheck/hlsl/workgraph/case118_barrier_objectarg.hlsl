// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Barrier is called with each node record and UAV type
// ==================================================================

struct RECORD
{
    uint value;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,1)]
[NodeDispatchGrid(256,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input)
{
   Barrier(input, 3);
}
// CHECK: define void @node01() {
// CHECK:   [[NODE01_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_NODE01_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE01_A]], %dx.types.NodeRecordInfo { i32 97, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE01_A]], i32 3)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256,1,1)]
void node02([MaxRecords(8)] GroupNodeInputRecords<RECORD> input)
{
   Barrier(input, 3);
}
// CHECK: define void @node02() {
// CHECK:   [[NODE02_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_NODE02_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE02_A]], %dx.types.NodeRecordInfo { i32 65, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE02_A]], i32 3)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("thread")]
void node03(RWThreadNodeInputRecord<RECORD> input)
{
   Barrier(input, 0);
}
// CHECK: define void @node03() {
// CHECK:   [[NODE03_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_NODE03_A:%[0-9]+]] =  call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE03_A]], %dx.types.NodeRecordInfo { i32 37, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE03_A]], i32 0)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256,1,1)]
void node04([MaxRecords(6)] RWGroupNodeInputRecords<RECORD> input)
{
   Barrier(input, 3);
}
// CHECK: define void @node04() {
// CHECK:   [[NODE04_A:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeInputRecordHandle(MetadataIdx)
// CHECK: [[ANN_NODE04_A:%[0-9]+]] =  call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE04_A]], %dx.types.NodeRecordInfo { i32 69, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE04_A]], i32 3)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,1)]
[NodeDispatchGrid(256,1,1)]
void node05([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   ThreadNodeOutputRecords<RECORD> outrec = outputs.GetThreadNodeOutputRecords(1);
   Barrier(outrec, 0);
}
// CHECK: define void @node05() {
// CHECK:   [[NODE05_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK:   [[ANN_NODE05_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[NODE05_A]], %dx.types.NodeInfo { i32 6, i32 4 })
// CHECK:   [[NODE05_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_NODE05_A]], i32 1, i1 true)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK: [[ANN_NODE05_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE05_B]], %dx.types.NodeRecordInfo { i32 38, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE05_B]], i32 0)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("thread")]
void node06([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   ThreadNodeOutputRecords<RECORD> outrec = outputs.GetThreadNodeOutputRecords(3);
   Barrier(outrec, 0);
}
// CHECK: define void @node06() {
// CHECK:   [[NODE06_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK:   [[ANN_NODE06_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[NODE06_A]], %dx.types.NodeInfo { i32 6, i32 4 })
// CHECK:   [[NODE06_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_NODE06_A]], i32 3, i1 true)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK:   [[ANN_NODE06_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE06_B]], %dx.types.NodeRecordInfo { i32 38, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE06_B]], i32 0)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(256,1,3)]
void node07([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   GroupNodeOutputRecords<RECORD> outrec = outputs.GetGroupNodeOutputRecords(1);
   Barrier(outrec, 3);
}
// CHECK: define void @node07() {
// CHECK:   [[NODE07_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK:   [[ANN_NODE07_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[NODE07_A]], %dx.types.NodeInfo { i32 6, i32 4 })
// CHECK:   [[NODE07_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_NODE07_A]], i32 1, i1 false)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK: [[ANN_NODE07_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE07_B]], %dx.types.NodeRecordInfo { i32 70, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE07_B]], i32 3)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node08([MaxRecords(5)] NodeOutput<RECORD> outputs)
{
   GroupNodeOutputRecords<RECORD> outrec = outputs.GetGroupNodeOutputRecords(4);
   Barrier(outrec, 3);
}
// CHECK: define void @node08() {
// CHECK:   [[NODE08_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0)  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK:   [[ANN_NODE08_A:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[NODE08_A]], %dx.types.NodeInfo { i32 6, i32 4 })
// CHECK:   [[NODE08_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_NODE08_A]], i32 4, i1 false)  ; AllocateNodeOutputRecords(output,numRecords,perThread)
// CHECK:   [[ANN_NODE08_B:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[NODE08_B]], %dx.types.NodeRecordInfo { i32 70, i32 4 })
// CHECK:   call void @dx.op.barrierByNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_NODE08_B]], i32 3)  ; BarrierByNodeRecordHandle(object,SemanticFlags)

RWBuffer<float> obj09;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node09()
{
   Barrier(obj09, 3);
}
// CHECK: define void @node09()
// CHECK:   [[NODE09_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?obj09@@3V?$RWBuffer@M@@A", align 4
// CHECK:   [[NODE09_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE09_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE09_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE09_B]], %dx.types.ResourceProperties { i32 4106, i32 265 })  ; AnnotateHandle(res,props)  resource: RWTypedBuffer<F32>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE09_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)


RWTexture1D<float4> obj10;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node10()
{
   Barrier(obj10, 3);
}
// CHECK: define void @node10()
// CHECK:   [[NODE10_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?obj10@@3V?$RWTexture1D@V?$vector@M$03@@@@A", align 4
// CHECK:   [[NODE10_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE10_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE10_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE10_B]], %dx.types.ResourceProperties { i32 4097, i32 1033 })  ; AnnotateHandle(res,props)  resource: RWTexture1D<4xF32>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE10_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)


RWTexture1DArray<float4> obj11;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node11()
{
   Barrier(obj11, 3);
}
// CHECK: define void @node11()
// CHECK:   [[NODE11_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?obj11@@3V?$RWTexture1DArray@V?$vector@M$03@@@@A", align 4
// CHECK:   [[NODE11_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE11_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE11_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE11_B]], %dx.types.ResourceProperties { i32 4102, i32 1033 })  ; AnnotateHandle(res,props)  resource: RWTexture1DArray<4xF32>

RWTexture2D<float> obj12;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node12()
{
   Barrier(obj12, 3);
}
// CHECK: define void @node12()
// CHECK:   [[NODE12_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?obj12@@3V?$RWTexture2D@M@@A", align 4
// CHECK:   [[NODE12_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE12_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE12_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE12_B]], %dx.types.ResourceProperties { i32 4098, i32 265 }) ; AnnotateHandle(res,props) resource: RWTexture2D<F32>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE12_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)

RWTexture2DArray<float> obj13;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node13()
{
   Barrier(obj13, 3);
}
// CHECK: define void @node13()
// CHECK:   [[NODE13_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle* @"\01?obj13@@3V?$RWTexture2DArray@M@@A", align 4
// CHECK:   [[NODE13_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE13_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE13_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE13_B]], %dx.types.ResourceProperties { i32 4103, i32 265 }) ; AnnotateHandle(res,props) resource: RWTexture2DArray<F32>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE13_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)

RWTexture3D<float> obj14;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node14()
{
   Barrier(obj14, 3);
}
// CHECK: define void @node14()
// CHECK:   [[NODE14_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle*  @"\01?obj14@@3V?$RWTexture3D@M@@A", align 4
// CHECK:   [[NODE14_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE14_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE14_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE14_B]], %dx.types.ResourceProperties { i32 4100, i32 265 }) ; AnnotateHandle(res,props) resource: RWTexture3D<F32>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE14_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)

RWStructuredBuffer<RECORD> obj15;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node15()
{
   Barrier(obj15, 3);
}
// CHECK: define void @node15()
// CHECK:   [[NODE15_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle*  @"\01?obj15@@3V?$RWStructuredBuffer@URECORD@@@@A", align 4
// CHECK:   [[NODE15_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE15_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE15_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE15_B]], %dx.types.ResourceProperties { i32 4620, i32 4 }) ; AnnotateHandle(res,props) resource: RWStructuredBuffer<stride=4>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE15_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)

RWByteAddressBuffer obj16;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node16()
{
   Barrier(obj16, 3);
}
// CHECK: define void @node16()
// CHECK:   [[NODE16_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle*  @"\01?obj16@@3URWByteAddressBuffer@@A", align 4
// CHECK:   [[NODE16_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE16_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE16_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE16_B]], %dx.types.ResourceProperties { i32 4107, i32 0 }) ; AnnotateHandle(res,props) resource: RWByteAddressBuffer
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE16_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)

AppendStructuredBuffer<RECORD> obj17;
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(256,1,4)]
[NodeDispatchGrid(256,1,1)]
void node17()
{
   Barrier(obj17, 3);
}
// CHECK: define void @node17()
// CHECK:   [[NODE17_A:%[0-9]+]] = load %dx.types.Handle, %dx.types.Handle*  @"\01?obj17@@3V?$AppendStructuredBuffer@URECORD@@@@A", align 4
// CHECK:   [[NODE17_B:%[0-9]+]] = call %dx.types.Handle @dx.op.createHandleForLib.dx.types.Handle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE17_A]])  ; CreateHandleForLib(Resource)
// CHECK:   [[NODE17_C:%[0-9]+]] = call %dx.types.Handle @dx.op.annotateHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE17_B]], %dx.types.ResourceProperties { i32 4620, i32 4 }) ; AnnotateHandle(res,props) resource: RWStructuredBuffer<stride=4>
// CHECK:   call void @dx.op.barrierByMemoryHandle(i32 {{[0-9]+}}, %dx.types.Handle [[NODE17_C]], i32 3)  ; BarrierByMemoryHandle(object,SemanticFlags)
