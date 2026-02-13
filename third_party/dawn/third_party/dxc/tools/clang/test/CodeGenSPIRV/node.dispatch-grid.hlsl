// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 external %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Broadcasting launch node with dispatch grid defined in shader

struct INPUT_NOGRID
{
  uint textureIndex;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2,3,2)]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node001_dispatchgrid_shader(DispatchNodeInputRecord<INPUT_NOGRID> input)
{
}

// CHECK: OpEntryPoint GLCompute [[SHADER:%[0-9A-Za-z_]*]]
// CHECK-DAG: OpExecutionMode [[SHADER]] LocalSize 1024 1 1
// CHECK-DAG: OpExecutionModeId [[SHADER]] StaticNumWorkgroupsAMDX [[U2:%[0-9A-Za-z_]*]]
// CHECK-SAME: [[U3:%[^ ]*]] [[U2]]
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U2]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U3]] = OpConstant [[UINT]] 3
// CHECK: OpReturn
