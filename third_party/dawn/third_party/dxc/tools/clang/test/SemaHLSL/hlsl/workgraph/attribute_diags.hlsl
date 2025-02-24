// RUN: %dxc -Tlib_6_8 -verify %s

// NodeDispatchGrid and NodeMaxDispatchGrid may only be used with Broadcasting launch nodes

[Shader("node")]
[NodeLaunch("coalescing")]           // expected-note +{{Launch type defined here}}
[NodeDispatchGrid(4, 4, 2)]          // expected-error {{'nodedispatchgrid' may only be used with broadcasting nodes}}
[NumThreads(32, 1, 1)]
void node01()
{ }

[Shader("node")]
[NodeLaunch("thread")]               // expected-note +{{Launch type defined here}}
[NodeDispatchGrid(8, 4, 2)]          // expected-error {{'nodedispatchgrid' may only be used with broadcasting nodes}}
void node02()
{ }

[Shader("node")]
[NodeLaunch("coalescing")]           // expected-note +{{Launch type defined here}}
[NodeMaxDispatchGrid(8, 8, 8)]       // expected-error {{'nodemaxdispatchgrid' may only be used with broadcasting nodes}}
[NumThreads(32, 1, 1)]
void node03()
{ }

[Shader("node")]
[NodeLaunch("thread")]               // expected-note +{{Launch type defined here}}
[NodeMaxDispatchGrid(256, 8, 8)]     // expected-error {{'nodemaxdispatchgrid' may only be used with broadcasting nodes}}
void node04()
{ }

// NodeDispatchGrid and NodeMaxDispatchGrid may not both be used

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(256, 64, 32)]   // expected-error {{Node shader 'node05' may not use both 'nodemaxdispatchgrid' and 'nodedispatchgrid'}}
[NodeDispatchGrid(32, 1, 1)]         // expected-note  {{nodedispatchgrid defined here}}
[NumThreads(32, 1, 1)]
void node05()                        // expected-error {{Broadcasting node shader 'node05' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
{ }

// One of NodeDispatchGrid or NodeMaxDispatchGrid must be specified for a Broadcasting node

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(32, 1, 1)]
void node06()                        // expected-error {{Broadcasting node shader 'node06' must have either the NodeDispatchGrid or NodeMaxDispatchGrid attribute}}
{ }

// NodeTrackRWInputRecordSharing must appear on the actual input record used if FinishedCrossGroupSharing may be called,
// and the attribute may not be inherited

struct [NodeTrackRWInputSharing] TrackedRecord {
  float4 f;
};

struct SharedRecord {
  float4 f;
};

struct InheritedRecord : TrackedRecord {
  float4 f;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node10(RWDispatchNodeInputRecord<TrackedRecord> input)
{
  input.FinishedCrossGroupSharing(); // no error as NodeTrackRWInputSharing is present
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node11(RWDispatchNodeInputRecord<SharedRecord> input)
{
  input.FinishedCrossGroupSharing(); // expected-error {{Use of FinishedCrossGroupSharing() requires NodeTrackRWInputSharing attribute to be specified on the record struct type}}
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
// The [NodeTrackRWInputSharing] attribute is not inherited
void node12(RWDispatchNodeInputRecord<InheritedRecord> input)
{
  input.FinishedCrossGroupSharing(); // expected-error {{Use of FinishedCrossGroupSharing() requires NodeTrackRWInputSharing attribute to be specified on the record struct type}}
}

// The chain of nodes may not exceed 32, which includes any recursive nodes.
// This means NodeMaxRecursionDepth may never be more than 32.

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
[NodeMaxRecursionDepth(33)] // expected-error {{NodeMaxRecursionDepth may not exceed 32}}
void node13(RWDispatchNodeInputRecord<SharedRecord> input)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
void node14() // expected-error {{Node shader 'node14' with broadcasting launch type requires 'numthreads' attribute}}
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
void node15() // expected-error {{Node shader 'node15' with coalescing launch type requires 'numthreads' attribute}}
{ }

[Shader("node")]
[NodeLaunch("thread")] // expected-note {{Launch type defined here}}
[NumThreads(1024,1,1)] // expected-error {{Thread launch nodes must have a thread group size of (1,1,1)}}
void node16()
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1025,1,1)] // expected-warning {{Group size of 1025 (1025 * 1 * 1) is outside of valid range [1..1024] - attribute will be ignored}}
void node17() // expected-error {{Node shader 'node17' with coalescing launch type requires 'numthreads' attribute}}
{ }

[Shader("node")]
[NumThreads(128,8,2)] // expected-warning {{Group size of 2048 (128 * 8 * 2) is outside of valid range [1..1024] - attribute will be ignored}}
[NodeLaunch("coalescing")]
void node18() // expected-error {{Node shader 'node18' with coalescing launch type requires 'numthreads' attribute}}
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node19(
  [MaxRecordsSharedWith(foo)] // expected-error {{attribute 'maxrecordssharedwith' may only be used with output nodes}}
  [AllowSparseNodes]          // expected-error {{attribute 'allowsparsenodes' may only be used with output nodes}}
  [NodeArraySize(1)]          // expected-error {{attribute 'NodeArraySize' may only be used with output nodes}}
  DispatchNodeInputRecord<SharedRecord> nodeInput,
  [MaxRecords(23)] NodeOutput<SharedRecord> foo)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node20(
  [MaxRecordsSharedWith(foo)] // expected-error {{only one of MaxRecords or MaxRecordsSharedWith may be specified to the same parameter.}}
  [AllowSparseNodes]
  [NodeArraySize(12)]         // expected-error {{[NodeArraySize(12)] conflict: [UnboundedSparseNodes] implies [NodeArraySize(-1)]}}
  [UnboundedSparseNodes]      // expected-note {{conflicting attribute is here}}
  [MaxRecords(23)]            // expected-note {{conflicting attribute is here}}
  NodeOutputArray<SharedRecord> foo)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node21(
  [MaxRecords(23)]            // expected-error {{attribute MaxRecords must only be used with node outputs or coalescing launch inputs}}
  [UnboundedSparseNodes]      // expected-error {{attribute 'unboundedsparsenodes' may only be used with output nodes}}
  DispatchNodeInputRecord<SharedRecord> nodeInput,
  [MaxRecords(23)] NodeOutputArray<SharedRecord> foo)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node22(
  [NodeArraySize(15)]         // expected-error {{attribute 'NodeArraySize' may only be used with node output arrays (NodeOutputArray or EmptyNodeOutputArray)}}
  [AllowSparseNodes]
  NodeOutput<SharedRecord> foo)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node23(
  [UnboundedSparseNodes]      // expected-error {{attribute 'unboundedsparsenodes' may only be used with node output arrays (NodeOutputArray or EmptyNodeOutputArray)}}
  NodeOutput<SharedRecord> foo)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node24(
  NodeOutput<SharedRecord> foo,
  [UnboundedSparseNodes]      // expected-error {{attribute 'unboundedsparsenodes' may only be used with output nodes}}
  [AllowSparseNodes]          // expected-error {{attribute 'allowsparsenodes' may only be used with output nodes}}
  [MaxRecordsSharedWith(foo)] // expected-error {{attribute 'maxrecordssharedwith' may only be used with output nodes}}
  [NodeArraySize(0xFFFFFFFF)] // expected-error {{attribute 'NodeArraySize' may only be used with output nodes}}
  uint3 gtid : SV_GroupThreadID)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(32, 1, 1)]
[NumThreads(32, 1, 1)]
void node25(
  [MaxRecords(23)]            // expected-error {{attribute 'MaxRecords' may only be used with output nodes or input record objects}}
  uint3 gtid : SV_GroupThreadID)
{ }
