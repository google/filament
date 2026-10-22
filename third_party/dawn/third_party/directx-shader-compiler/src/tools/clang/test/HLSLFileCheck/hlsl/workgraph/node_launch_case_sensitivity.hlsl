// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Check that validation errors are generated when the node launch
// attribute has a valid type with invalid casing in the string.
// ==================================================================

struct RECORD {
 uint a;
};

[Shader("node")]
// CHECK: error: attribute 'NodeLaunch' must have one of these values: broadcasting,coalescing,thread
[NodeLaunch("BrOaDcasting")] 
[NodeDispatchGrid(1, 1, 1)] 
[NumThreads(1,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input) { }

[Shader("node")]
// CHECK: error: attribute 'NodeLaunch' must have one of these values: broadcasting,coalescing,thread
[NodeLaunch("coAleSCing")]
[NumThreads(1,1,1)]
void node02(GroupNodeInputRecords<RECORD> input) { }

[Shader("node")]
// CHECK: error: attribute 'NodeLaunch' must have one of these values: broadcasting,coalescing,thread
[NodeLaunch("tHREAD")]
[NumThreads(1,1,1)]
void node03(ThreadNodeInputRecord<RECORD> input) { }
