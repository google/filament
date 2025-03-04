// RUN: %dxc -Tlib_6_8 -verify %s
// NodeDispatchGrid and NodeMaxDispatchGrid validation diagnostics:
// - the x, y, z, component values must be in the range 1 to 2^16 - 1 (65,535) inclusive
// - the product x * y * z must not exceed 2^24 - 1 (16,777,215)
// - a warning should be generated for 2nd and subsequent occurances of these attributes
// - node with NodeMaxDispatchGrid must have a input record with SV_DispatchGrid semantics
//   on exactly one field

struct MyStruct {
    float a;
    uint3 grid : SV_DispatchGrid;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 1, 1)]
[NumThreads(32, 1, 1)]
void node01()
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 65536, 1)]       // expected-error {{'NodeDispatchGrid' Y component value must be between 1 and 65,535 (2^16-1) inclusive}}
[NumThreads(32, 1, 1)]
void node02()
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 0)]           // expected-error {{'NodeDispatchGrid' Z component value must be between 1 and 65,535 (2^16-1) inclusive}}
[NumThreads(32, 1, 1)]
void node03()
{ }

static const int x = 1<<16;
static const uint y = 4;
static const int z = 0;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(x, 256, 256)]       // expected-error {{'NodeDispatchGrid' X component value must be between 1 and 65,535 (2^16-1) inclusive}}
[NumThreads(32, 1, 1)]
void node04()
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256, 256, 256)]     // expected-error {{'NodeDispatchGrid' X * Y * Z product may not exceed 16,777,215 (2^24-1)}}
[NumThreads(32, 1, 1)]
void node05()
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(64, 4, 2)]          // expected-warning {{attribute 'NodeDispatchGrid' is already applied}}
[NodeDispatchGrid(64, 4, 2)]
[NumThreads(32, 1, 1)]
void node06()
{}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(65535, 1, 1)]
[NumThreads(32, 1, 1)]
void node11(DispatchNodeInputRecord<MyStruct> input)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 65536, 1)]    // expected-error {{'NodeMaxDispatchGrid' Y component value must be between 1 and 65,535 (2^16-1) inclusive}}
[NumThreads(32, 1, 1)]
void node12(DispatchNodeInputRecord<MyStruct> input)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 1, 0)]        // expected-error {{'NodeMaxDispatchGrid' Z component value must be between 1 and 65,535 (2^16-1) inclusive}}
[NumThreads(32, 1, 1)]
void node13(DispatchNodeInputRecord<MyStruct> input)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, y, z)]        // expected-error {{'NodeMaxDispatchGrid' Z component value must be between 1 and 65,535 (2^16-1) inclusive}}
[NumThreads(32, 1, 1)]
void node14(DispatchNodeInputRecord<MyStruct> input)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(1, 65535, 257)]  // expected-error {{'NodeMaxDispatchGrid' X * Y * Z product may not exceed 16,777,215 (2^24-1)}}
[NumThreads(32, 1, 1)]
void node15(DispatchNodeInputRecord<MyStruct> input)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(256, 8, 8)]      // expected-warning {{attribute 'NodeMaxDispatchGrid' is already applied}}
[NodeMaxDispatchGrid(64, 1, 1)]
[NumThreads(32, 1, 1)]
void node16(DispatchNodeInputRecord<MyStruct> input)
{ }

[Shader("node")]
[NodeMaxDispatchGrid(3, 1, 1)]
[NumThreads(17, 1, 1)]
void node17()               // expected-error {{Broadcasting node shader 'node17' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
{ }

struct MyStruct2 {
    float3 b;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(256, 8, 8)]
[NumThreads(32, 1, 1)]
void node18(DispatchNodeInputRecord<MyStruct2> input)  // expected-error {{Broadcasting node shader 'node18' with NodeMaxDispatchGrid attribute must declare an input record containing a field with SV_DispatchGrid semantic}}
{ }

struct MyStruct3 {
    uint3 grid : SV_DispatchGrid;  // expected-note {{other SV_DispatchGrid defined here}}
    float3 b;
    int3 grid2 : SV_DispatchGrid;  // expected-error {{a field with SV_DispatchGrid has already been specified}}
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(256, 8, 8)]
[NumThreads(32, 1, 1)]
void node19(DispatchNodeInputRecord<MyStruct3> input)
{ }

struct A {
    float a;
    uint3 grid : SV_DispatchGrid;   // expected-note {{other SV_DispatchGrid defined here}}
};

struct B : A {
    float b;
};

struct C : B {
    float b;
    int3 grid2 : SV_DispatchGrid;   // expected-error {{a field with SV_DispatchGrid has already been specified}}
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(8, 4, 4)]      
[NumThreads(32, 1, 1)]
void node20(DispatchNodeInputRecord<C> input)
{ }

struct D {
  uint3 grid1 : SV_DispatchGrid;   // expected-note {{other SV_DispatchGrid defined here}}
};

struct E {
  D struct_field;
};

struct F : E {
  uint4 grid2 : SV_DispatchGrid;   // expected-error {{a field with SV_DispatchGrid has already been specified}}
  float data;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(32, 16, 1)]
[NumThreads(32, 1, 1)]
void node21(DispatchNodeInputRecord<F> input)
{ }
