// RUN: %dxc -spirv -Vd -Od -T lib_6_8 external -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// GetInputRecordCount() called with NodeInputRecordArray

RWBuffer<uint> buf0;

struct INPUT_RECORD
{
    uint textureIndex;
};

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node014_getinputrecordcount([MaxRecords(256)] GroupNodeInputRecords<INPUT_RECORD> inputs)
{
  uint numRecords = inputs.Count();
  buf0[0] = numRecords;
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK: OpNodePayloadArrayLengthAMDX [[UINT]]
