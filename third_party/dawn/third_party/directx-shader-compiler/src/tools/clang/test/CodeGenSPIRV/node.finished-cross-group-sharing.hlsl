// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// FinishedCrossGroupSharing() is called with RWDispatchNodeInputRecord

RWBuffer<uint> buf0;

struct [NodeTrackRWInputSharing] INPUT_RECORD
{
  uint value;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
void node037_finishedcrossgroupsharing(RWDispatchNodeInputRecord<INPUT_RECORD> input)
{
  bool b = input.FinishedCrossGroupSharing();
  buf0[0] = 0 ? b : 1;
}

// CHECK: OpName [[INPUT:%[^ ]*]] "input"
// CHECK: OpDecorate [[STRUCT:%[^ ]*]] TrackFinishWritingAMDX
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[STRUCT]] = OpTypeStruct [[UINT]]
// CHECK: [[ARR:%[^ ]*]] = OpTypeNodePayloadArrayAMDX [[STRUCT]]
// CHECK: [[PTR:%[^ ]*]] = OpTypePointer NodePayloadAMDX [[ARR]]
// CHECK: [[BOOL:%[^ ]*]] = OpTypeBool
// CHECK: [[INPUT]] = OpFunctionParameter [[PTR]]
// CHECK: OpFinishWritingNodePayloadAMDX [[BOOL]] [[INPUT]]
