// RUN: %dxc -spirv -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// IsValid() is invoked on NodeOutput

RWBuffer<uint> buf0;

struct RECORD
{
  uint value;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
void node129_nodeoutputisvalid_nodeoutput(NodeOutput<RECORD> output)
{
  buf0[0] = output.IsValid();
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: [[U0:%[^ ]*]] = OpConstant [[UINT]] 0
// CHECK: [[BOOL:%[^ ]*]] = OpTypeBool
// CHECK: OpIsNodePayloadValidAMDX [[BOOL]] %{{[^ ]*}} [[U0]]
