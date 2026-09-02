// RUN: %dxc -Tlib_6_8 -verify %s
// ==================================================================
// Errors are generated for writes to members of read-only records
// ==================================================================

struct RECORD
{
  uint3 a;
  bool b;
  uint3 grid : SV_DispatchGrid;
};

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1024,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input1)
{
  RECORD x;
  input1.Get() = x; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input1.Get().a = 11; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input1.Get().a[0] = 12; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input1.Get().a.z = 13; //expected-error{{read-only variable is not assignable}}
  input1.Get().b = false; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1024,1,1)]
void node02(RWDispatchNodeInputRecord<RECORD> input2)
{
  RECORD x;
  input2.Get() = x;
  input2.Get().a = 21;
  input2.Get().a[0] = 22;
  input2.Get().a.z = 23;
  input2.Get().b = true;
}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("coalescing")]
void node03([MaxRecords(3)] GroupNodeInputRecords<RECORD> input3)
{
  RECORD x;
  input3.Get() = x; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input3.Get(0).a = 31; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input3.Get(1).a[2] = 32; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input3.Get(2).a.x = 33; //expected-error{{read-only variable is not assignable}}
  input3[3].a = 34; //expected-error{{cannot assign to return value because function 'operator[]' returns a const value}}
  input3.Get().b = false; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input3[0].b = false; //expected-error{{cannot assign to return value because function 'operator[]' returns a const value}}
}

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node04([MaxRecords(4)] RWGroupNodeInputRecords<RECORD> input4)
{
  RECORD x;
  input4.Get() = x;
  input4.Get(1).a = 41;
  input4.Get(2).a[2] = 42;
  input4.Get(3).a.x = 43;
  input4.Get(1).a = 44;
  input4[0].b = true;
  input4.Get().b = true;
  input4.Get(0).b = true;
  input4[0].b = true;
}

[Shader("node")]
[NodeLaunch("thread")]
void node05(ThreadNodeInputRecord<RECORD> input5)
{
  RECORD x;
  input5.Get() = x; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input5.Get().a = 51; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input5.Get().a[0] = 52; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
  input5.Get().a.z = 53; //expected-error{{read-only variable is not assignable}}
  input5.Get().b = false; //expected-error{{cannot assign to return value because function 'Get' returns a const value}}
}

[Shader("node")]
[NodeLaunch("thread")]
void node06(RWThreadNodeInputRecord<RECORD> input6)
{
  RECORD x;
  input6.Get() = x;
  input6.Get().a = 61;
  input6.Get().a[0] = 62;
  input6.Get().a.z = 63;
  input6.Get().b = true;
}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1024,1,1)]
void node07(NodeOutput<RECORD> output7)
{
  RECORD x;
  output7.Get() = x; //expected-error{{no member named 'Get' in 'NodeOutput<RECORD>'}}
  output7.a = 71; //expected-error{{no member named 'a' in 'NodeOutput<RECORD>'}}
  output7[0].b = false; //expected-error{{type 'NodeOutput<RECORD>' does not provide a subscript operator}}
  output7.Get().a = 72; //expected-error{{no member named 'Get' in 'NodeOutput<RECORD>'}}
  output7.Get().b = false; //expected-error{{no member named 'Get' in 'NodeOutput<RECORD>'}}
}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1024,1,1)]
void node08([MaxRecords(8)] NodeOutputArray<RECORD> output8)
{
  RECORD x;
  output8.Get() = x; //expected-error{{no member named 'Get' in 'NodeOutputArray<RECORD>'}}
  output8.Get().a = 81; //expected-error{{no member named 'Get' in 'NodeOutputArray<RECORD>'}}
  output8[0].a = 82; //expected-error{{no member named 'a' in 'NodeOutput<RECORD>'}}
  output8.b = false; //expected-error{{no member named 'b' in 'NodeOutputArray<RECORD>'}}
  output8.Get().b = false; //expected-error{{no member named 'Get' in 'NodeOutputArray<RECORD>'}}
}

// expected-note@? +{{function 'Get' which returns const-qualified type 'const RECORD &' declared here}}
// expected-note@? +{{function 'operator[]' which returns const-qualified type 'const RECORD &' declared here}}

