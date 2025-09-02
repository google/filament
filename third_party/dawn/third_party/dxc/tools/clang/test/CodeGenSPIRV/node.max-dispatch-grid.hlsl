// RUN: %dxc -spirv -Vd -Od -T lib_6_8 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// Note: validation disabled until NodePayloadAMDX pointers are allowed
// as function arguments

// Broadcasting launch node with dispatch grid defined in input
// and max dispatch grid defined in the shader

struct INPUT_GRID
{
  uint3 DispatchGrid : SV_DispatchGrid;
  uint textureIndex;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeMaxDispatchGrid(2,3,4)]
[NumThreads(1024,1,1)]
void node002_dispatchgrid_input_maxdispatchgrid_shader(DispatchNodeInputRecord<INPUT_GRID> input)
{
}

// CHECK: OpEntryPoint GLCompute [[SHADER:%[^ ]*]] "node002_dispatchgrid_input_maxdispatchgrid_shader"
// CHECK-DAG: OpExecutionMode [[SHADER]] LocalSize 1024 1 1
// CHECK-DAG: OpExecutionModeId [[SHADER]] MaxNumWorkgroupsAMDX [[U2:%[^ ]*]] [[U3:%[^ ]*]] [[U4:%[0-9A-Za-z_]*]]
// CHECK: OpMemberDecorate %{{[^ ]*}} 0 PayloadDispatchIndirectAMDX
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U2]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U3]] = OpConstant [[UINT]] 3
// CHECK-DAG: [[U4]] = OpConstant [[UINT]] 4
// CHECK: OpReturn
