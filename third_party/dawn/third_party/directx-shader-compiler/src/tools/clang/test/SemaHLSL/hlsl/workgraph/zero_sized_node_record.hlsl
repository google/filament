// RUN: %dxc -Tlib_6_8 -verify %s
// ==================================================================
// zero-sized-node-record (expected error)
// An error diagnostic is generated for a zero sized record used in
// a node input/output record declaration.
// ==================================================================

struct EMPTY { // expected-note +{{zero sized record defined here}}
};

struct EMPTY2 { // expected-note +{{zero sized record defined here}}
  EMPTY empty;
};

struct EMPTY3 { // expected-note +{{zero sized record defined here}}
  EMPTY2 empty;
};

template<typename T> struct EmptyTemplate { // expected-note +{{zero sized record defined here}}
  // no fields here
};

struct EMPTY4 { // expected-note +{{zero sized record defined here}}
  EmptyTemplate<EMPTY3> empty;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(64, 1, 1)]
void node01(DispatchNodeInputRecord<EMPTY> input) // expected-error {{record used in DispatchNodeInputRecord may not have zero size}} expected-error {{Broadcasting node shader 'node01' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
{}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(64, 1, 1)]
void node02(RWDispatchNodeInputRecord<EMPTY> input) // expected-error {{record used in RWDispatchNodeInputRecord may not have zero size}} expected-error {{Broadcasting node shader 'node02' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
{}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node03(GroupNodeInputRecords<EMPTY> input) // expected-error {{record used in GroupNodeInputRecords may not have zero size}}
{}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node04(RWGroupNodeInputRecords<EMPTY> input) // expected-error {{record used in RWGroupNodeInputRecords may not have zero size}}
{}

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void node05(ThreadNodeInputRecord<EMPTY> input) // expected-error {{record used in ThreadNodeInputRecord may not have zero size}}
{}

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void node06(RWThreadNodeInputRecord<EMPTY2> input) // expected-error {{record used in RWThreadNodeInputRecord may not have zero size}}
{}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeDispatchGrid(64, 1, 1)]
void node07(NodeOutput<EMPTY> output) // expected-error {{record used in NodeOutput may not have zero size}}
{}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(64, 1, 1)]
// expected-error@+1 {{Broadcasting node shader 'node08' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
void node08(NodeOutputArray<EMPTY3> output) // expected-error {{record used in NodeOutputArray may not have zero size}}
{}

[Shader("node")]
[NodeLaunch("thread")]
void node09()
{
  ThreadNodeOutputRecords<EMPTY> outrec1; // expected-error {{record used in ThreadNodeOutputRecords may not have zero size}}

  GroupNodeOutputRecords<EMPTY3> outrec2; // expected-error {{record used in GroupNodeOutputRecords may not have zero size}}

  ThreadNodeOutputRecords<EmptyTemplate<int> > outrec3; // expected-error {{record used in ThreadNodeOutputRecords may not have zero size}}

  GroupNodeOutputRecords<EMPTY4> outrec4; // expected-error {{record used in GroupNodeOutputRecords may not have zero size}}
}
