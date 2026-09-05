// RUN: %dxc -spirv -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s

// NumThreads

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
[NodeIsProgramEntry]
void node010_thread_numthreads_shader()
{
}

// CHECK: OpEntryPoint GLCompute [[SHADER:%[0-9A-Za-z_]*]]
// CHECK: OpExecutionMode [[SHADER]] LocalSize 1 1 1
// CHECK: OpReturn
