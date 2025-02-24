// RUN: %dxc -T lib_6_8 -verify %s

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(65535, 65535, 65535)] // expected-error {{'NodeDispatchGrid' X * Y * Z product may not exceed 16,777,215 (2^24-1)}}
[NumThreads(1, 1, 1)]
void myNode() { }
