// RUN: %dxc -Tlib_6_8 -Wno-unused-value -verify %s

// Legal cases, no diagnostics expected for this file.
// expected-no-diagnostics

struct RECORD { uint a; };
RWBuffer<uint> buf0;
static uint i = 7;

// Barriers not requiring visible group, compatible with any shader stage.
[noinline] export
void DeviceBarriers() {
  Barrier(0, 0);

  Barrier(ALL_MEMORY, 0);
  Barrier(ALL_MEMORY, DEVICE_SCOPE);
  Barrier(ALL_MEMORY, 2 + 2);

  Barrier(UAV_MEMORY, 0);
  Barrier(UAV_MEMORY, DEVICE_SCOPE);

  // No diagnostic for non-constant expressions.
  Barrier(ALL_MEMORY, i + i);
  Barrier(i + i, 0);
}

// Barriers requiring visible group.
[noinline] export
void GroupBarriers() {
  Barrier(0, GROUP_SYNC);

  Barrier(ALL_MEMORY, GROUP_SYNC);
  Barrier(ALL_MEMORY, GROUP_SCOPE);
  Barrier(ALL_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  Barrier(ALL_MEMORY, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);
  Barrier(ALL_MEMORY, 1 + 2 + 4);

  Barrier(UAV_MEMORY, GROUP_SYNC);
  Barrier(UAV_MEMORY, GROUP_SCOPE);
  Barrier(UAV_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  Barrier(UAV_MEMORY, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);

  Barrier(GROUP_SHARED_MEMORY, 0);
  Barrier(GROUP_SHARED_MEMORY, GROUP_SYNC);
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE);
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE | GROUP_SYNC);
}

// Node barriers not requiring visible group, compatible with all launch modes.
[noinline] export
void NodeBarriers() {
  Barrier(NODE_INPUT_MEMORY, 0);
  Barrier(NODE_INPUT_MEMORY, DEVICE_SCOPE);

  Barrier(NODE_OUTPUT_MEMORY, 0);
  Barrier(NODE_OUTPUT_MEMORY, DEVICE_SCOPE);
}

// Node barriers requiring visible group.
[noinline] export
void GroupNodeBarriers() {
  Barrier(NODE_INPUT_MEMORY, GROUP_SYNC);
  Barrier(NODE_INPUT_MEMORY, GROUP_SCOPE);
  Barrier(NODE_INPUT_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  Barrier(NODE_INPUT_MEMORY, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);

  Barrier(NODE_OUTPUT_MEMORY, GROUP_SYNC);
  Barrier(NODE_OUTPUT_MEMORY, GROUP_SCOPE);
  Barrier(NODE_OUTPUT_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  Barrier(NODE_OUTPUT_MEMORY, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);
}

//////////////////////////////////////////////////////////////////////////////
// Entry Points

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(64,1,1)]
[NodeDispatchGrid(1, 1, 1)]
void node01(RWDispatchNodeInputRecord<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {
  DeviceBarriers();
  GroupBarriers();
  NodeBarriers();
  GroupNodeBarriers();

  Barrier(buf0, 0);
  Barrier(buf0, GROUP_SYNC);
  Barrier(buf0, GROUP_SCOPE);
  Barrier(buf0, DEVICE_SCOPE);
  Barrier(buf0, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);

  Barrier(input, 0);
  Barrier(input, GROUP_SYNC);
  Barrier(input, GROUP_SCOPE);
  Barrier(input, DEVICE_SCOPE);
  Barrier(input, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);

  GroupNodeOutputRecords<RECORD> groupRec = output.GetGroupNodeOutputRecords(1);
  Barrier(groupRec, 0);
  Barrier(groupRec, GROUP_SYNC);
  Barrier(groupRec, GROUP_SCOPE);
  Barrier(groupRec, GROUP_SCOPE | GROUP_SYNC);
  // Not allowed:
  // Barrier(groupRec, DEVICE_SCOPE);
  groupRec.OutputComplete();

  ThreadNodeOutputRecords<RECORD> threadRec = output.GetThreadNodeOutputRecords(1);
  Barrier(threadRec, 0);
  Barrier(threadRec, GROUP_SYNC);
  // Not allowed:
  // Barrier(threadRec, GROUP_SCOPE);
  // Barrier(threadRec, DEVICE_SCOPE);
  threadRec.OutputComplete();
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(64,1,1)]
void node01([MaxRecords(64)] RWGroupNodeInputRecords<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {
  DeviceBarriers();
  GroupBarriers();
  NodeBarriers();
  GroupNodeBarriers();

  Barrier(buf0, 0);
  Barrier(buf0, GROUP_SYNC);
  Barrier(buf0, GROUP_SCOPE);
  Barrier(buf0, DEVICE_SCOPE);
  Barrier(buf0, DEVICE_SCOPE | GROUP_SCOPE | GROUP_SYNC);

  Barrier(input, 0);
  Barrier(input, GROUP_SYNC);
  Barrier(input, GROUP_SCOPE);
  Barrier(input, GROUP_SCOPE | GROUP_SYNC);
  // Not allowed:
  // Barrier(input, DEVICE_SCOPE);

  GroupNodeOutputRecords<RECORD> groupRec = output.GetGroupNodeOutputRecords(1);
  Barrier(groupRec, 0);
  Barrier(groupRec, GROUP_SCOPE);
  Barrier(groupRec, GROUP_SYNC);
  Barrier(groupRec, GROUP_SCOPE | GROUP_SYNC);
  // Not allowed:
  // Barrier(groupRec, DEVICE_SCOPE);
  groupRec.OutputComplete();

  ThreadNodeOutputRecords<RECORD> threadRec = output.GetThreadNodeOutputRecords(1);
  Barrier(threadRec, 0);
  Barrier(threadRec, GROUP_SYNC);
  // Not allowed:
  // Barrier(threadRec, GROUP_SCOPE);
  // Barrier(threadRec, DEVICE_SCOPE);
  threadRec.OutputComplete();
}

[Shader("node")]
[NodeLaunch("thread")]
void node02(RWThreadNodeInputRecord<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {
  DeviceBarriers();
  NodeBarriers();
  // Not allowed:
  // GroupBarriers();
  // GroupNodeBarriers();

  Barrier(buf0, 0);
  Barrier(buf0, DEVICE_SCOPE);
  // Not allowed:
  // Barrier(buf0, GROUP_SYNC);
  // Barrier(buf0, GROUP_SCOPE);

  Barrier(input, 0);
  // Not allowed:
  // Barrier(input, GROUP_SYNC);
  // Barrier(input, GROUP_SCOPE);
  // Barrier(input, DEVICE_SCOPE);

  GroupNodeOutputRecords<RECORD> groupRec = output.GetGroupNodeOutputRecords(1);
  Barrier(groupRec, 0);
  // Not allowed:
  // Barrier(groupRec, GROUP_SCOPE);
  // Barrier(groupRec, GROUP_SYNC);
  // Barrier(groupRec, DEVICE_SCOPE);
  groupRec.OutputComplete();

  ThreadNodeOutputRecords<RECORD> threadRec = output.GetThreadNodeOutputRecords(1);
  Barrier(threadRec, 0);
  // Not allowed:
  // Barrier(threadRec, GROUP_SYNC);
  // Barrier(threadRec, GROUP_SCOPE);
  // Barrier(threadRec, DEVICE_SCOPE);
  threadRec.OutputComplete();
}
