// RUN: %dxc -T lib_6_8 -verify %s
// ==================================================================
// Broadcasting launch node with class input record
// ==================================================================

// expected-no-diagnostics

class ClassInputRecord
{
  uint a;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2,3,2)]
[NumThreads(1024,1,1)]
void node01(DispatchNodeInputRecord<ClassInputRecord> input)
{ }
