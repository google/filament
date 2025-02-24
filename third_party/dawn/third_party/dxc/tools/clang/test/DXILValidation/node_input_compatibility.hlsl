// RUN: %dxc -T lib_6_8 %s
// ==================================================================
// Check that validation errors are generated when an input record
// has a type that is invalid with the launch mode.
// The validation test replaces each of the (valid) input types in
// turn to provoke the error diagnostics.
// ==================================================================

struct RECORD {
 uint a;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input) { }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node02(GroupNodeInputRecords<RECORD> input) { }

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void node03(ThreadNodeInputRecord<RECORD> input) { }
