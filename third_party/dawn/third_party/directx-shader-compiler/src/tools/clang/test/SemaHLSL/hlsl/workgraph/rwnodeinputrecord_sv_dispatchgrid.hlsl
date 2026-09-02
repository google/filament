// RUN: %dxc -Tlib_6_8 -verify %s

// Check that a RWNodeInputRecord field that has the SV_DispatchGrid semantic
// is not assignable.

struct RECORD
{
  uint3 a;
  uint3 b : SV_DispatchGrid;
};

//==============================================================================
// Check non-assignable fields

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node01(RWGroupNodeInputRecords<RECORD> input)
{
  input.Get().a = 11;
  input.Get().a.yz = 12;
  input.Get().a[0] = 13;
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(2,1,1)]
void node02(GroupNodeInputRecords<RECORD> input)
{
  input.Get().a = 21; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input.Get().b = 22; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(4,1,1)]
void node03([MaxRecords(4)] GroupNodeInputRecords<RECORD> input)
{
  input[1].a = 31; //expected-error{{cannot assign to return value because function 'operator[]' returns a const value}}
  input[1].b = 32; //expected-error{{cannot assign to return value because function 'operator[]' returns a const value}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(8,1,1)]
void node04(const RWGroupNodeInputRecords<RECORD> input) 
{
  input.Get().a = 41; // expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input.Get().b = 42; // expected-error{{cannot assign to return value because function 'Get' returns a const value}}
}

// expected-note@? +{{function 'Get' which returns const-qualified type 'const RECORD &' declared here}}
// expected-note@? +{{function 'operator[]' which returns const-qualified type 'const RECORD &' declared here}}
