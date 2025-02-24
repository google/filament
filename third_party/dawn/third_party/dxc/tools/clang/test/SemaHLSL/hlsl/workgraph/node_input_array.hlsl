// RUN: %dxc -Tlib_6_8 -verify %s
// Node input types cannot be used as parameters to entry functions

struct Record {
  uint a;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(65535, 1, 1)]
[NodeIsProgramEntry]
[NumThreads(32, 1, 1)]
// expected-error@+2 {{Broadcasting node shader 'node01' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
// expected-error@+1 {{entry parameter of type 'GroupNodeInputRecords<Record> [9]' may not be an array}}
void node01(GroupNodeInputRecords<Record> input[9])
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(65535, 1, 1)]
[NodeIsProgramEntry]
[NumThreads(32, 1, 1)]
// expected-error@+2 {{Broadcasting node shader 'node02' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
// expected-error@+1 {{entry parameter of type 'RWGroupNodeInputRecords<Record> [9]' may not be an array}}
void node02(RWGroupNodeInputRecords<Record> input[9])
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(128,1,1)]
// expected-error@+1 {{entry parameter of type 'DispatchNodeInputRecord<Record> [4]' may not be an array}}
void node03(DispatchNodeInputRecord<Record> input[4])
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(128,1,1)]
// expected-error@+1 {{entry parameter of type 'RWDispatchNodeInputRecord<Record> [4]' may not be an array}}
void node04(RWDispatchNodeInputRecord<Record> input[4])
{ }

[Shader("node")]
[NodeLaunch("thread")]
// expected-error@+1 {{entry parameter of type 'ThreadNodeInputRecord<Record> [7]' may not be an array}}
void node05(ThreadNodeInputRecord<Record> input[7])
{ }

[Shader("node")]
[NodeLaunch("thread")]
// expected-error@+1 {{entry parameter of type 'RWThreadNodeInputRecord<Record> []' may not be an array}}
void node06(RWThreadNodeInputRecord<Record> input[])
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(128,1,1)]
// expected-error@+1 {{entry parameter of type 'EmptyNodeInput [10]' may not be an array}}
void node07(EmptyNodeInput input[10])
{ }
