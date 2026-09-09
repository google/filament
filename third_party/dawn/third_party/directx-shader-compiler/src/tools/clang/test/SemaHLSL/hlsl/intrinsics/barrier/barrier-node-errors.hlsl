// RUN: %dxc -Tlib_6_8 -verify %s

struct RECORD { uint a; };
RWBuffer<uint> buf0;
static uint i = 7;

// Errors expected when called without visible group.
[noinline] export
void GroupBarriers() {
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(0, GROUP_SYNC);

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, DEVICE_SCOPE | GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(ALL_MEMORY, 1 + 2 + 4);

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(UAV_MEMORY, DEVICE_SCOPE | GROUP_SCOPE);

  // expected-error@+1{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, 0);
  // expected-error@+2{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SYNC);
  // expected-error@+2{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE);
  // expected-error@+2{{GROUP_SHARED_MEMORY specified for Barrier operation when context has no visible group}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(GROUP_SHARED_MEMORY, GROUP_SCOPE);
}

// Errors expected when called without visible group.
[noinline] export
void GroupNodeBarriers() {
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_INPUT_MEMORY, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_INPUT_MEMORY, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_INPUT_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_INPUT_MEMORY, DEVICE_SCOPE | GROUP_SCOPE);

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_OUTPUT_MEMORY, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_OUTPUT_MEMORY, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_OUTPUT_MEMORY, GROUP_SCOPE | GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(NODE_OUTPUT_MEMORY, DEVICE_SCOPE | GROUP_SCOPE);
}

// expected-note@+7{{entry function defined here}}
// expected-note@+6{{entry function defined here}}
// expected-note@+5{{entry function defined here}}
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(64,1,1)]
[NodeDispatchGrid(1, 1, 1)]
void node01(RWDispatchNodeInputRecord<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {

  GroupNodeOutputRecords<RECORD> groupRec = output.GetGroupNodeOutputRecords(1);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(groupRec, DEVICE_SCOPE);
  groupRec.OutputComplete();

  ThreadNodeOutputRecords<RECORD> threadRec = output.GetThreadNodeOutputRecords(1);
  // expected-error@+1{{GROUP_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(threadRec, GROUP_SCOPE);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(threadRec, DEVICE_SCOPE);
  threadRec.OutputComplete();
}

// expected-note@+7{{entry function defined here}}
// expected-note@+6{{entry function defined here}}
// expected-note@+5{{entry function defined here}}
// expected-note@+4{{entry function defined here}}
[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(64,1,1)]
void node01([MaxRecords(64)] RWGroupNodeInputRecords<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {

  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(input, DEVICE_SCOPE);

  GroupNodeOutputRecords<RECORD> groupRec = output.GetGroupNodeOutputRecords(1);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(groupRec, DEVICE_SCOPE);
  groupRec.OutputComplete();

  ThreadNodeOutputRecords<RECORD> threadRec = output.GetThreadNodeOutputRecords(1);
  // expected-error@+1{{GROUP_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(threadRec, GROUP_SCOPE);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(threadRec, DEVICE_SCOPE);
  threadRec.OutputComplete();
}

// expected-note@+40{{entry function defined here}}
// expected-note@+39{{entry function defined here}}
// expected-note@+38{{entry function defined here}}
// expected-note@+37{{entry function defined here}}
// expected-note@+36{{entry function defined here}}
// expected-note@+35{{entry function defined here}}
// expected-note@+34{{entry function defined here}}
// expected-note@+33{{entry function defined here}}
// expected-note@+32{{entry function defined here}}
// expected-note@+31{{entry function defined here}}
// expected-note@+30{{entry function defined here}}
// expected-note@+29{{entry function defined here}}
// expected-note@+28{{entry function defined here}}
// expected-note@+27{{entry function defined here}}
// expected-note@+26{{entry function defined here}}
// expected-note@+25{{entry function defined here}}
// expected-note@+24{{entry function defined here}}
// expected-note@+23{{entry function defined here}}
// expected-note@+22{{entry function defined here}}
// expected-note@+21{{entry function defined here}}
// expected-note@+20{{entry function defined here}}
// expected-note@+19{{entry function defined here}}
// expected-note@+18{{entry function defined here}}
// expected-note@+17{{entry function defined here}}
// expected-note@+16{{entry function defined here}}
// expected-note@+15{{entry function defined here}}
// expected-note@+14{{entry function defined here}}
// expected-note@+13{{entry function defined here}}
// expected-note@+12{{entry function defined here}}
// expected-note@+11{{entry function defined here}}
// expected-note@+10{{entry function defined here}}
// expected-note@+9{{entry function defined here}}
// expected-note@+8{{entry function defined here}}
// expected-note@+7{{entry function defined here}}
// expected-note@+6{{entry function defined here}}
// expected-note@+5{{entry function defined here}}
// expected-note@+4{{entry function defined here}}
// expected-note@+3{{entry function defined here}}
[Shader("node")]
[NodeLaunch("thread")]
void node02(RWThreadNodeInputRecord<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {

  GroupBarriers();
  GroupNodeBarriers();

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(buf0, GROUP_SYNC);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(buf0, GROUP_SCOPE);

  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(input, GROUP_SYNC);
  // expected-error@+2{{GROUP_SCOPE specified for Barrier operation without applicable memory}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(input, GROUP_SCOPE);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(input, DEVICE_SCOPE);

  GroupNodeOutputRecords<RECORD> groupRec = output.GetGroupNodeOutputRecords(1);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(groupRec, GROUP_SCOPE);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(groupRec, GROUP_SYNC);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(groupRec, DEVICE_SCOPE);
  groupRec.OutputComplete();

  ThreadNodeOutputRecords<RECORD> threadRec = output.GetThreadNodeOutputRecords(1);
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(threadRec, GROUP_SYNC);
  // expected-error@+2{{GROUP_SCOPE specified for Barrier operation without applicable memory}}
  // expected-error@+1{{GROUP_SYNC or GROUP_SCOPE specified for Barrier operation when context has no visible group}}
  Barrier(threadRec, GROUP_SCOPE);
  // expected-error@+1{{DEVICE_SCOPE specified for Barrier operation without applicable memory}}
  Barrier(threadRec, DEVICE_SCOPE);
  threadRec.OutputComplete();
}

// Default launch type is broadcasting which has a visible group.
// It is OK to use GROUP_SYNC or GROUP_SCOPE.
[Shader("node")]
[NumThreads(64,1,1)]
[NodeDispatchGrid(1, 1, 1)]
void defaultBroadcastingLaunch(RWDispatchNodeInputRecord<RECORD> input,
            [MaxRecords(11)] NodeOutput<RECORD> output) {
  Barrier(UAV_MEMORY, GROUP_SYNC);
  Barrier(UAV_MEMORY, GROUP_SCOPE);
  Barrier(UAV_MEMORY, GROUP_SCOPE|GROUP_SYNC);
}
