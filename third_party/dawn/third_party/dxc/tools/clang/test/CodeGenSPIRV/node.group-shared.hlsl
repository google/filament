// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Check that group shared memory is allowed from a work graph node

struct Record
{
    uint index;
};

groupshared uint testLds[512];

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1,1,1)]
void firstNode(DispatchNodeInputRecord<Record> inputData)
{
    testLds[inputData.Get().index] = 99;
}

// CHECK: OpReturn

