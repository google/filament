// RUN: %dxc -Tlib_6_8 -HV 2021 -verify %s

struct RECORD
{
  uint a;
  bool b;
  uint3 grid : SV_DispatchGrid;
};

struct RECORD2
{
  uint a;
  bool b;
};

struct [NodeTrackRWInputSharing] TRACKED_RECORD
{
  uint a;
};

//==============================================================================
// Check Get[GroupShared]NodeOutput[Array]() intrinsics don't match with invalid
// parameter types.

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node2_01([MaxRecords(5)] EmptyNodeOutput output)
{
  // GetGroupNodeOutputRecords() is called on an EmptyNodeOutput
  output.GetGroupNodeOutputRecords(1);   /* expected-error {{no member named 'GetGroupNodeOutputRecords' in 'EmptyNodeOutput'}} */
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node2_02([MaxRecords(5)] EmptyNodeOutput output)
{
  // GetThreadNodeOutputRecords() is called on an EmptyNodeOutput
  output.GetThreadNodeOutputRecords(3);   /* expected-error {{no member named 'GetThreadNodeOutputRecords' in 'EmptyNodeOutput'}} */
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
void node2_05(DispatchNodeInputRecord<RECORD> input)
{
  // GetGroupNodeOutputRecords() is called on a DispatchNodeInputRecord<>
  input.GetGroupNodeOutputRecords(1);   /* expected-error {{no member named 'GetGroupNodeOutputRecords' in 'DispatchNodeInputRecord<RECORD>'}} */
}

template<typename T>
struct FakeNodeOutput {
  int h;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node2_06(FakeNodeOutput<RECORD> output)
{
  // GetGroupNodeOutputRecords() is called on a type that is like NodeOutput<> to check INTRIN_COMPTYPE_FROM_NODEOUTPUT isn't fooled.
  GroupNodeOutputRecords<RECORD> outrec = output.GetGroupNodeOutputRecords(1); /* expected-error {{no member named 'GetGroupNodeOutputRecords' in 'FakeNodeOutput<RECORD>'}} */
}

//==============================================================================
// Check invalid initialization of *NodeOutputRecords

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node3_01(NodeOutput<RECORD> output)
{
  // Initializing a GroupNodeOutputRecords<RECORD2> from NodeOutput<RECORD>::GetNodeOutput()
  GroupNodeOutputRecords<RECORD2> outrec = output.GetGroupNodeOutputRecords(5); /* expected-error {{cannot initialize a variable of type 'GroupNodeOutputRecords<RECORD2>' with an rvalue of type 'GroupNodeOutputRecords<RECORD>'}} */
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node3_02(NodeOutput<RECORD> output)
{
  // Initializing a ThreadNodeOutputRecords<RECORD2> from NodeOutput<RECORD>::ThreadNodeOutputRecords()
  ThreadNodeOutputRecords<RECORD2> outrec = output.GetThreadNodeOutputRecords(5); /* expected-error {{cannot initialize a variable of type 'ThreadNodeOutputRecords<RECORD2>' with an rvalue of type 'ThreadNodeOutputRecords<RECORD>'}} */
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node3_03(NodeOutput<RECORD> output)
{
  // Initializing a ThreadNodeOutputRecords<RECORD> from GetGroupNodeOutputRecords() 
  ThreadNodeOutputRecords<RECORD> outrec = output.GetGroupNodeOutputRecords(1); /* expected-error {{cannot initialize a variable of type 'ThreadNodeOutputRecords<RECORD>' with an rvalue of type 'GroupNodeOutputRecords<RECORD>'}} */
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node3_04(NodeOutput<RECORD> output)
{
  // Initializing a GroupNodeOutputRecords<RECORD> from GetThreadNodeOutputRecords() 
  GroupNodeOutputRecords<RECORD> outrec = output.GetThreadNodeOutputRecords(1); /* expected-error {{cannot initialize a variable of type 'GroupNodeOutputRecords<RECORD>' with an rvalue of type 'ThreadNodeOutputRecords<RECORD>'}} */
}

//==============================================================================
// Check FinishedCrossGroupSharing only available for RWDispatchNodeInputRecord
// in Broadcasting launch nodes

[Shader("node")]
[NumThreads(8,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(8,1,1)]
void node4_01(RWDispatchNodeInputRecord<TRACKED_RECORD> input) {
  input.FinishedCrossGroupSharing(); // no error 
}


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(8,1,1)]
void node4_02(DispatchNodeInputRecord<RECORD> input) {
  input.FinishedCrossGroupSharing(); // expected-error {{no member named 'FinishedCrossGroupSharing' in 'DispatchNodeInputRecord<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node4_03(GroupNodeInputRecords<RECORD> input)
{
  bool foo = input.FinishedCrossGroupSharing(); // expected-error {{no member named 'FinishedCrossGroupSharing' in 'GroupNodeInputRecords<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node4_04(RWGroupNodeInputRecords<RECORD> input)
{
  bool foo = input.FinishedCrossGroupSharing(); // expected-error {{no member named 'FinishedCrossGroupSharing' in 'RWGroupNodeInputRecords<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
void node4_05(ThreadNodeInputRecord<RECORD> input)
{
  input.FinishedCrossGroupSharing(); // expected-error {{no member named 'FinishedCrossGroupSharing' in 'ThreadNodeInputRecord<RECORD>'}}
}


[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
void node4_06(RWThreadNodeInputRecord<RECORD> input)
{
  input.FinishedCrossGroupSharing(); // expected-error {{no member named 'FinishedCrossGroupSharing' in 'RWThreadNodeInputRecord<RECORD>'}}
}

struct R
{
  uint gidx;
  uint3 gtid;
};

[Shader("node")]
[NodeLaunch("coalescing")]
[numthreads(4,4,4)]
void node01(RWGroupNodeInputRecords<R> input,
 uint gidx: SV_GroupIndex,
  uint3 gtid: SV_GroupThreadID,
 uint vid : SV_VertexID ) // expected-error {{Invalid system value semantic 'SV_VertexID' for launchtype 'Coalescing'}}
 // TODO: verify error after https://github.com/microsoft/DirectXShaderCompiler/issues/5768 got fixed.
{
}
