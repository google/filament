// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Check that a barrier can be used on a groupshared object from a
// work graph node

groupshared uint Test;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void firstNode()
{
  Test = 1;
  AllMemoryBarrierWithGroupSync();
}

// CHECK: OpReturn
