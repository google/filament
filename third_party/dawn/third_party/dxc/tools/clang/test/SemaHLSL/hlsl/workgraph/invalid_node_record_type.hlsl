// RUN: %dxc -Tlib_6_8 -HV 2021 -verify %s

struct RECORD
{
  uint a;
  bool b;
  float4 c;
  matrix<float,3,3> d;
};

struct BAD_RECORD
{
  uint a;
  RECORD rec;
  SamplerState s; // expected-note+ {{'SamplerState' field declared here}}
};

struct BAD_RECORD2
{
  BAD_RECORD bad;
};

typedef matrix<float,2,2> f2x2;

typedef SamplerState MySamplerState;

typedef BAD_RECORD2 MyBadRecord;

//==============================================================================
// Check diagnostics for the various node input/output types

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
// expected-error@+2 {{'int' is not valid as a node record type - struct/class required}}
// expected-error@+1 {{node shader 'node01' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
void node01(DispatchNodeInputRecord<int> input) 
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(8,1,1)]
void node02(GroupNodeInputRecords<float4> input) // expected-error {{'float4' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node03(ThreadNodeInputRecord<int2x2> input) // expected-error {{'int2x2' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
// expected-error@+2 {{'int [8]' is not valid as a node record type - struct/class required}}
// expected-error@+1 {{node shader 'node04' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
void node04(RWDispatchNodeInputRecord<int[8]> input)
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(2,1,1)]
void node05(RWGroupNodeInputRecords<SamplerState> input) // expected-error {{'SamplerState' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node06(RWThreadNodeInputRecord<matrix<float,4,4> > input) // expected-error {{'matrix<float, 4, 4>' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node07(RWThreadNodeInputRecord<f2x2> input) // expected-error {{'f2x2' (aka 'matrix<float, 2, 2>') is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node08(ThreadNodeInputRecord<BAD_RECORD> input) // expected-error {{object 'SamplerState' is not allowed in node records}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node09(ThreadNodeInputRecord<BAD_RECORD[4]> input) // expected-error {{'BAD_RECORD [4]' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node10(RWThreadNodeInputRecord<BAD_RECORD2> input) // expected-error {{object 'SamplerState' is not allowed in node records}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node11(NodeOutput<BAD_RECORD> input) // expected-error {{object 'SamplerState' is not allowed in node records}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node12(NodeOutputArray<MyBadRecord> output) // expected-error {{object 'SamplerState' is not allowed in node records}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node13(NodeOutputArray<DispatchNodeInputRecord<RECORD> > output) // expected-error {{'DispatchNodeInputRecord<RECORD>' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node14(ThreadNodeInputRecord<matrix<float,4,4> > input) // expected-error {{'matrix<float, 4, 4>' is not valid as a node record type - struct/class required}}
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
// expected-error@+2 {{'RaytracingAccelerationStructure' is not valid as a node record type - struct/class}}
// expected-error@+1 {{node shader 'node15' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
void node15(NodeOutputArray<RaytracingAccelerationStructure> output)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeMaxDispatchGrid(8,1,1)]
// expected-error@+1 {{node shader 'node16' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
void node16()
{
  GroupNodeOutputRecords<int> outrec1; // expected-error {{'int' is not valid as a node record type - struct/class required}}

  ThreadNodeOutputRecords<f2x2> outrec2; // expected-error {{'f2x2' (aka 'matrix<float, 2, 2>') is not valid as a node record type - struct/class required}}

  GroupNodeOutputRecords<MyBadRecord> outrec3; // expected-error {{object 'SamplerState' is not allowed in node records}}

  ThreadNodeOutputRecords<SamplerState> outrec4; // expected-error {{'SamplerState' is not valid as a node record type - struct/class required}}
}


template<typename T> struct MyTemplateStruct {
  T MyField; // expected-note+ {{'SamplerState' field declared here}}
};

struct MyNestedTemplateStruct {
  MyTemplateStruct<SamplerState> a;
};

// This should be accepted without error
[Shader("node")]
[NodeLaunch("thread")]
void node17(ThreadNodeInputRecord<MyTemplateStruct<int> > input)
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node18(ThreadNodeInputRecord<MyTemplateStruct<SamplerState> > input) // expected-error {{object 'SamplerState' is not allowed in node records}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node19(RWThreadNodeInputRecord<MyNestedTemplateStruct> input) // expected-error {{object 'SamplerState' is not allowed in node records}}
{ }
