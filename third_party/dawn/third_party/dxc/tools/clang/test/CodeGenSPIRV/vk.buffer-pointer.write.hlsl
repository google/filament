// RUN: %dxc -spirv -T ps_6_0 -E MainPs %s | FileCheck %s

// CHECK: OpEntryPoint Fragment [[FUN:%[_0-9A-Za-z]*]] "MainPs" [[OUT:%[_0-9A-Za-z]*]]

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

// CHECK: [[FLOAT:%[_0-9A-Za-z]*]] = OpTypeFloat 32
// CHECK-DAG: [[F0:%[_0-9A-Za-z]*]] = OpConstant [[FLOAT]] 0
// CHECK-DAG: [[F1:%[_0-9A-Za-z]*]] = OpConstant [[FLOAT]] 1
// CHECK: [[V4FLOAT:%[_0-9A-Za-z]*]] = OpTypeVector [[FLOAT]] 4
// CHECK-DAG: [[CV4FLOAT:%[_0-9A-Za-z]*]] = OpConstantComposite [[V4FLOAT]] [[F1]] [[F0]] [[F0]] [[F0]]
// CHECK: [[SINT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK-DAG: [[S0:%[_0-9A-Za-z]*]] = OpConstant [[SINT]] 0
// CHECK-DAG: [[S1:%[_0-9A-Za-z]*]] = OpConstant [[SINT]] 1
// CHECK: [[GLOBALS:%[_0-9A-Za-z]*]] = OpTypeStruct [[V4FLOAT]] [[V4FLOAT]] [[V4FLOAT]]
// CHECK: [[PGLOBALS:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[GLOBALS]]
// CHECK: [[PC:%[_0-9A-Za-z]*]] = OpTypeStruct [[PGLOBALS]]
// CHECK: [[PPC:%[_0-9A-Za-z]*]] = OpTypePointer PushConstant [[PC]]
// CHECK: [[PV4FLOAT1:%[_0-9A-Za-z]*]] = OpTypePointer Output [[V4FLOAT]]
// CHECK: [[PPGLOBALS:%[_0-9A-Za-z]*]] = OpTypePointer PushConstant [[PGLOBALS]]
// CHECK: [[PV4FLOAT2:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[V4FLOAT]]
// CHECK: [[GPC:%[_0-9A-Za-z]*]] = OpVariable [[PPC]] PushConstant
// CHECK-DAG: [[OUT]] = OpVariable [[PV4FLOAT1]] Output

float4 MainPs(void) : SV_Target0
{
      float4 vTest = float4(1.0,0.0,0.0,0.0);
      g_PushConstants.m_nBufferDeviceAddress.Get().g_vTestFloat4 = vTest;
      vk::BufferPointer<float,4>(0xdeadbeefull).Get() = 4.5f;
      return vTest;
}

// CHECK: [[FUN]] = OpFunction
// CHECK: [[X1:%[_0-9A-Za-z]*]] = OpAccessChain [[PPGLOBALS]] [[GPC]] [[S0]]
// CHECK: [[X2:%[_0-9A-Za-z]*]] = OpLoad [[PGLOBALS]] [[X1]]
// CHECK: [[X3:%[_0-9A-Za-z]*]] = OpAccessChain [[PV4FLOAT2]] [[X2]] [[S1]]
// CHECK: OpStore [[X3]] [[CV4FLOAT]] Aligned 16
// CHECK: [[TEMP_PTR:%[_0-9A-Za-z]*]] = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_float %ulong_3735928559
// CHECK: OpStore [[TEMP_PTR]] %float_4_5 Aligned 4
// CHECK: OpStore [[OUT]] [[CV4FLOAT]]
// CHECK: OpFunctionEnd
