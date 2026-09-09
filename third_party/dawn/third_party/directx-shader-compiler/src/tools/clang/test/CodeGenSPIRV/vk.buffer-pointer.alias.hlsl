// RUN: %dxc -spirv -Od -T ps_6_0 -E MainPs %s | FileCheck %s

struct Globals_s
{
    float4 g_vSomeConstantA;
    float4 g_vTestFloat4;
    float4 g_vSomeConstantB;
};

typedef vk::BufferPointer<Globals_s> Globals_p;

struct TestPushConstant_t
{
    Globals_p m_nBufferDeviceAddress;
};

[[vk::push_constant]] TestPushConstant_t g_PushConstants;

cbuffer cbuf {
    [[vk::aliased_pointer]] Globals_p bp;
}

// CHECK: OpDecorate [[BP0:%[_0-9A-Za-z]*]] AliasedPointer
// CHECK: OpDecorate [[BP1:%[_0-9A-Za-z]*]] AliasedPointer
// CHECK: OpDecorate [[BP:%[_0-9A-Za-z]*]] AliasedPointer
// CHECK: [[FLOAT:%[_0-9A-Za-z]*]] = OpTypeFloat 32
// CHECK-DAG: [[F1:%[_0-9A-Za-z]*]] = OpConstant [[FLOAT]] 1
// CHECK-DAG: [[F0:%[_0-9A-Za-z]*]] = OpConstant [[FLOAT]] 0
// CHECK: [[V4FLOAT:%[_0-9A-Za-z]*]] = OpTypeVector [[FLOAT]] 4
// CHECK: [[V4C:%[_0-9A-Za-z]*]] = OpConstantComposite [[V4FLOAT]] [[F1]] [[F0]] [[F0]] [[F0]]
// CHECK: [[INT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK-DAG: [[I0:%[_0-9A-Za-z]*]] = OpConstant [[INT]] 0
// CHECK-DAG: [[I1:%[_0-9A-Za-z]*]] = OpConstant [[INT]] 1
// CHECK: [[GS:%[_0-9A-Za-z]*]] = OpTypeStruct [[V4FLOAT]] [[V4FLOAT]] [[V4FLOAT]]
// CHECK: [[PGS:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[GS]]
// CHECK: [[TT:%[_0-9A-Za-z]*]] = OpTypeStruct [[PGS]]
// CHECK: [[PTT:%[_0-9A-Za-z]*]] = OpTypePointer PushConstant [[TT]]
// CHECK: [[PFV4FLOAT:%[_0-9A-Za-z]*]] = OpTypePointer Function [[V4FLOAT]]
// CHECK: [[PPGS:%[_0-9A-Za-z]*]] = OpTypePointer PushConstant [[PGS]]
// CHECK: [[PBV4FLOAT:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[V4FLOAT]]

void f([[vk::aliased_pointer]] Globals_p bp) {
}

float4 MainPs(void) : SV_Target0
{
    float4 vTest = float4(1.0,0.0,0.0,0.0);
    [[vk::aliased_pointer]] Globals_p bp0 = Globals_p(g_PushConstants.m_nBufferDeviceAddress);
    [[vk::aliased_pointer]] Globals_p bp1 = Globals_p(g_PushConstants.m_nBufferDeviceAddress);
    bp0.Get().g_vTestFloat4 = vTest;
    f(bp0);
    return bp1.Get().g_vTestFloat4; // Returns float4(1.0,0.0,0.0,0.0)
}

// CHECK: [[GP:%[_0-9A-Za-z]*]] = OpVariable [[PTT]] PushConstant
// CHECK: [[VTEST:%[0-9A-Za-z]*]] = OpVariable [[PFV4FLOAT]] Function
// CHECK: OpStore [[VTEST]] [[V4C]]
// CHECK: [[X1:%[_0-9A-Za-z]*]] = OpAccessChain [[PPGS]] [[GP]] [[I0]]
// CHECK: [[X2:%[_0-9A-Za-z]*]] = OpLoad %_ptr_PhysicalStorageBuffer_Globals_s [[X1]]
// CHECK: OpStore [[BP0]] [[X2]]
// CHECK: [[X3:%[_0-9A-Za-z]*]] = OpAccessChain [[PPGS]] [[GP]] [[I0]]
// CHECK: [[X4:%[_0-9A-Za-z]*]] = OpLoad [[PGS]] [[X3]]
// CHECK: OpStore [[BP1]] [[X4]]
// CHECK: [[X5:%[_0-9A-Za-z]*]] = OpLoad [[V4FLOAT]] [[VTEST]]
// CHECK: [[X6:%[_0-9A-Za-z]*]] = OpLoad [[PGS]] [[BP0]]
// CHECK: [[X7:%[_0-9A-Za-z]*]] = OpAccessChain [[PBV4FLOAT]] [[X6]] [[I1]]
// CHECK: OpStore [[X7]] [[X5]] Aligned 16
// CHECK: [[X8:%[_0-9A-Za-z]*]] = OpLoad [[PGS]] [[BP1]]
// CHECK: [[X9:%[_0-9A-Za-z]*]] = OpAccessChain [[PBV4FLOAT]] [[X8]] [[I1]]
// CHECK: [[X10:%[_0-9A-Za-z]*]] = OpLoad [[V4FLOAT]] [[X9]] Aligned 16
// CHECK: OpReturnValue [[X10]]

