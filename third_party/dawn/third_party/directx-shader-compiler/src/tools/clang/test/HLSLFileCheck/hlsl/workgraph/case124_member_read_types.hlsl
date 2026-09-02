// RUN: %dxc -T lib_6_8 -enable-16bit-types %s | FileCheck %s
// ==================================================================
// Read access of members of input/output record with different type
// sizes - we check the function specializations generated
// ==================================================================

RWBuffer<uint> buf0;

struct RECORD
{
  half h;
  float f;
  double d;
  bool b;
  uint16_t i16;
  int i;
  int64_t i64;
  uint64_t u64;
  float3 f3;
};

// CHECK: %[[RECORD:struct\.RECORD.*]] = type { half, float, double, i32, i16, i32, i64, i64, [3 x float] }

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node01(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().h;
}
// CHECK: define void @node01() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node02(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().f;
  
}
// CHECK: define void @node02() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node03(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().d;
  
}
// CHECK: define void @node03() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node04(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().b;
  
}
// CHECK: define void @node04() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node05(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().i16;
  
}
// CHECK: define void @node05() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node06(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().i;
  
}
// CHECK: define void @node06() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node07(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().i64;
  
}
// CHECK: define void @node07() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node08(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = input.Get().u64;
  
}
// CHECK: define void @node08() {
// CHECK: {{%[0-9]+}} = call %[[RECORD]] addrspace(6)* @dx.op.getNodeRecordPtr.[[RECORD]](i32 {{[0-9]+}}, %dx.types.NodeRecordHandle {{%[0-9]+}}, i32 0)
