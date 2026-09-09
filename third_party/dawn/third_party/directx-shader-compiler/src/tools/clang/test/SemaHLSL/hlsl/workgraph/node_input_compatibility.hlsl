// RUN: %dxc -Tlib_6_8 -verify %s
// ==================================================================
// Errors are generated for node inputs that are not compatible with
// the launch type
// ==================================================================

struct RECORD
{
  uint a;
};

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]                         // expected-note  {{Launch type defined here}}
void node01(GroupNodeInputRecords<RECORD> input)     // expected-error {{'GroupNodeInputRecords' may not be used with broadcasting nodes (only DispatchNodeInputRecord or RWDispatchNodeInputRecord)}}
{ }

[NodeDispatchGrid(64,1,1)]
[Shader("node")]
[NumThreads(1024,1,1)]
void node02(RWGroupNodeInputRecords<RECORD> input)   // expected-error {{'RWGroupNodeInputRecords' may not be used with broadcasting nodes (only DispatchNodeInputRecord or RWDispatchNodeInputRecord)}}
{ }

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]                         // expected-note  {{Launch type defined here}}
void node03(ThreadNodeInputRecord<RECORD> input)     // expected-error {{'ThreadNodeInputRecord' may not be used with broadcasting nodes (only DispatchNodeInputRecord or RWDispatchNodeInputRecord)}}
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]                         // expected-note  {{Launch type defined here}}
[NodeDispatchGrid(64,1,1)]
[NumThreads(1024,1,1)]
void node04(RWThreadNodeInputRecord<RECORD> input)   // expected-error {{'RWThreadNodeInputRecord' may not be used with broadcasting nodes (only DispatchNodeInputRecord or RWDispatchNodeInputRecord)}}
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]                         // expected-note  {{Launch type defined here}}
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
void node05(EmptyNodeInput input)                    // expected-error {{'EmptyNodeInput' may not be used with broadcasting nodes (only DispatchNodeInputRecord or RWDispatchNodeInputRecord)}}
{ }

[Shader("node")]
[NumThreads(128,1,1)]
[NodeLaunch("coalescing")]                           // expected-note  {{Launch type defined here}}
void node06(DispatchNodeInputRecord<RECORD> input)   // expected-error {{'DispatchNodeInputRecord' may not be used with coalescing nodes (only GroupNodeInputRecords, RWGroupNodeInputRecords, or EmptyNodeInput)}}
{ }

[NodeLaunch("coalescing")]                           // expected-note  {{Launch type defined here}}
[Shader("node")]
[NumThreads(128,1,1)]
void node07(RWDispatchNodeInputRecord<RECORD> input) // expected-error {{'RWDispatchNodeInputRecord' may not be used with coalescing nodes (only GroupNodeInputRecords, RWGroupNodeInputRecords, or EmptyNodeInput)}}
{ }

[Shader("node")]
[NumThreads(128,1,1)]
[NodeLaunch("coalescing")]                           // expected-note  {{Launch type defined here}}
void node08(ThreadNodeInputRecord<RECORD> input)     // expected-error {{'ThreadNodeInputRecord' may not be used with coalescing nodes (only GroupNodeInputRecords, RWGroupNodeInputRecords, or EmptyNodeInput)}}
{ }

[NodeLaunch("coalescing")]                           // expected-note  {{Launch type defined here}}
[Shader("node")]
[NumThreads(128,1,1)]
void node09(RWThreadNodeInputRecord<RECORD> input)   // expected-error {{'RWThreadNodeInputRecord' may not be used with coalescing nodes (only GroupNodeInputRecords, RWGroupNodeInputRecords, or EmptyNodeInput)}}
{ }

[Shader("node")]
[NodeLaunch("thread")]                               // expected-note  {{Launch type defined here}}
void node10(DispatchNodeInputRecord<RECORD> input)   // expected-error {{'DispatchNodeInputRecord' may not be used with thread nodes (only ThreadNodeInputRecord or RWThreadNodeInputRecord)}}
{ }

[NodeLaunch("thread")]                               // expected-note  {{Launch type defined here}}
[Shader("node")]
void node11(RWDispatchNodeInputRecord<RECORD> input) // expected-error {{'RWDispatchNodeInputRecord' may not be used with thread nodes (only ThreadNodeInputRecord or RWThreadNodeInputRecord)}}
{ }

[Shader("node")]
[NodeLaunch("thread")]                               // expected-note  {{Launch type defined here}}
void node12(GroupNodeInputRecords<RECORD> input)     // expected-error {{'GroupNodeInputRecords' may not be used with thread nodes (only ThreadNodeInputRecord or RWThreadNodeInputRecord)}}
{ }

[NodeLaunch("thread")]                               // expected-note  {{Launch type defined here}}
[Shader("node")]
void node13(RWGroupNodeInputRecords<RECORD> input)   // expected-error {{'RWGroupNodeInputRecords' may not be used with thread nodes (only ThreadNodeInputRecord or RWThreadNodeInputRecord)}}
{ }

[Shader("node")]
[NodeLaunch("thread")]                               // expected-note  {{Launch type defined here}}
void node14(EmptyNodeInput input)                    // expected-error {{'EmptyNodeInput' may not be used with thread nodes (only ThreadNodeInputRecord or RWThreadNodeInputRecord)}}
{ }

[Shader("node")]
[NodeDispatchGrid(1,1,1)]
[NumThreads(1,1,1)]
void node15(RWDispatchNodeInputRecord<RECORD> input,
            DispatchNodeInputRecord<RECORD> input2)  // expected-error {{Node shader 'node15' may not have more than one input record}}
{ }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node16([MaxRecords(8)] RWGroupNodeInputRecords<RECORD> input,
            EmptyNodeInput empty)                    // expected-error {{Node shader 'node16' may not have more than one input record}}
{ }

[Shader("node")]
[NodeLaunch("thread")]
void node17(ThreadNodeInputRecord<RECORD> input,
            NodeOutput<RECORD> output,
            RWThreadNodeInputRecord<RECORD> input2)  // expected-error {{Node shader 'node17' may not have more than one input record}}
{ }
