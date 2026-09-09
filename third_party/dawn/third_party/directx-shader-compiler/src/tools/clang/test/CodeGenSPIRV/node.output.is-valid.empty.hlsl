// RUN: %dxc -spirv -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// NodeOutputIsValid() is called with EmptyNodeOutput

RWBuffer<uint> buf0;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
void node131_nodeoutputisvalid_emptynodeoutput(EmptyNodeOutput output)
{
  buf0[0] = output.IsValid();
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK: [[BOOL:%[^ ]*]] = OpTypeBool
// CHECK: OpIsNodePayloadValidAMDX [[BOOL]] %{{[^ ]*}} [[U0]]
