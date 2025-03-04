// RUN: %dxc -T lib_6_8 -HV 2021 %s -verify

struct [NodeTrackRWInputSharing] TRACKED_RECORD
{
  uint a;
};

//==============================================================================
// Check Get[GroupShared]NodeOutput[Array]() intrinsics don't match with invalid
// parameter types.

[Shader("node")]
[NumThreads(8,1,1)]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(8,1,1)]
// expected-error@+1{{NodeTrackRWInputSharing attribute cannot be applied to Input Records that are not RWDispatchNodeInputRecord}}
void node4_01(DispatchNodeInputRecord<TRACKED_RECORD> input) {
}
