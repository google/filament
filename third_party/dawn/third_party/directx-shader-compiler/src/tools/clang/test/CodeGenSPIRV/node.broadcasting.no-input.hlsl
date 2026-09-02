// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Broadcasting launch node with no input

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(3,4,5)]
[NumThreads(6,7,1)]
[NodeIsProgramEntry]
void node070_broadcasting_noinput()
{
}

// CHECK: OpReturn

