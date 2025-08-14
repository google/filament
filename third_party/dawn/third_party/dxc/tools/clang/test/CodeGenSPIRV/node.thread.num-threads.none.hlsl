// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// Thread launch node without NumThreads specified should use a
// default of (1,1,1)

[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
void node011_thread_numthreads_none()
{
}

// CHECK: OpEntryPoint GLCompute [[SHADER:%[0-9A-Za-z_]*]]
// CHECK: OpExecutionMode [[SHADER]] LocalSize 1 1 1
// CHECK: OpReturn
