// RUN: %dxc -Tlib_6_8 -verify %s
// NodeOutputArray and EmptyNodeOutputArray are provided in place of
// NodeOutput and EmptyNodeOutput native array parameters.
// Check error diagnostics are produced for such native array parameters.

struct Record {
  uint a;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 1, 1)]
[NodeIsProgramEntry]
[NumThreads(32, 1, 1)]
// expected-error@+2 {{entry parameter of type 'NodeOutput<Record> [9]' may not be an array}}
// expected-note@+1 {{'NodeOutput' cannot be used as an array; did you mean 'NodeOutputArray'?}}
void node01(NodeOutput<Record> output[9])
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 1, 1)]
[NumThreads(32, 1, 1)]
// expected-error@+2 {{entry parameter of type 'EmptyNodeOutput [4]' may not be an array}}
// expected-note@+1 {{'EmptyNodeOutput' cannot be used as an array; did you mean 'EmptyNodeOutputArray'?}}
void node02(EmptyNodeOutput output[4])
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 1, 1)]
[NodeIsProgramEntry]
[NumThreads(32, 1, 1)]
// expected-error@+1 {{entry parameter of type 'NodeOutputArray<Record> [9]' may not be an array}}
void node03(NodeOutputArray<Record> output[9])
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 1, 1)]
[NumThreads(32, 1, 1)]
// expected-error@+1 {{entry parameter of type 'EmptyNodeOutputArray [4]' may not be an array}}
void node04(EmptyNodeOutputArray output[4])
{ }
