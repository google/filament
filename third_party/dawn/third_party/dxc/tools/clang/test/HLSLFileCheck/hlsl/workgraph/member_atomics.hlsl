// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s
// ==================================================================
// Test using atomics on node record members for cmpxchg and binops
// ==================================================================

struct RECORD
{
  uint ival;
  float fval;
};

// CHECK: define void @node01
[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(32,1,1)]
[NodeLaunch("broadcasting")]
void node01(RWDispatchNodeInputRecord<RECORD> input1)
{
  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: bitcast float addrspace(6)* %{{[0-9A-Za-z_]*}} to i32 addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 0, i32 1123477094
  InterlockedCompareStoreFloatBitwise(input1.Get().fval, 0.0, 123.45);

  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 111, i32 222
  // CHECK: atomicrmw add i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 333
  InterlockedCompareStore(input1.Get().ival, 111, 222);
  InterlockedAdd(input1.Get().ival, 333);
}

// CHECK: define void @node02
[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node02([MaxRecords(4)]RWGroupNodeInputRecords<RECORD> input2)
{
  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: bitcast float addrspace(6)* %{{[0-9A-Za-z_]*}} to i32 addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 0, i32 1123477094
  InterlockedCompareStoreFloatBitwise(input2[0].fval, 0.0, 123.45);

  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 111, i32 222
  // CHECK: atomicrmw add i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 333
  InterlockedCompareStore(input2[1].ival, 111, 222);
  InterlockedAdd(input2[2].ival, 333);
}

// CHECK: define void @node03
[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(32,1,1)]
[NodeLaunch("broadcasting")]
void node03(NodeOutput<RECORD> output3)
{
  GroupNodeOutputRecords<RECORD> outrec = output3.GetGroupNodeOutputRecords(1);
  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: bitcast float addrspace(6)* %{{[0-9A-Za-z_]*}} to i32 addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 0, i32 1123477094
  InterlockedCompareStoreFloatBitwise(outrec.Get().fval, 0.0, 123.45);

  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 111, i32 222
  // CHECK: atomicrmw add i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 333
  InterlockedCompareStore(outrec.Get().ival, 111, 222);
  InterlockedAdd(outrec.Get().ival, 333);
}

// CHECK: define void @node04
[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("coalescing")]
void node04([MaxRecords(5)] NodeOutput<RECORD> outputs4)
{
   ThreadNodeOutputRecords<RECORD> outrec = outputs4.GetThreadNodeOutputRecords(1);
  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: bitcast float addrspace(6)* %{{[0-9A-Za-z_]*}} to i32 addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 0, i32 1123477094
  InterlockedCompareStoreFloatBitwise(outrec.Get().fval, 0.0, 123.45);

  // CHECK: getelementptr %struct.RECORD, %struct.RECORD addrspace(6)*
  // CHECK: cmpxchg i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 111, i32 222
  // CHECK: atomicrmw add i32 addrspace(6)* %{{[0-9A-Za-z_]*}}, i32 333
  InterlockedCompareStore(outrec.Get().ival, 111, 222);
  InterlockedAdd(outrec.Get().ival, 333);
}
