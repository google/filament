// RUN: %dxc -T cs_6_8 -E node_compute %s | FileCheck %s

// CHECK: error: shader attribute type 'node' conflicts with shader attribute type 'compute'
// CHECK: note: conflicting attribute is here

[Shader("node")]
[Shader("compute")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(9,5,6)]
void node_compute() { }
