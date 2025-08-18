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

// CHECK: [[SINT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK-DAG: [[S0:%[_0-9A-Za-z]*]] = OpConstant [[SINT]] 0
// CHECK-DAG: [[S1:%[_0-9A-Za-z]*]] = OpConstant [[SINT]] 1
// CHECK: [[FLOAT:%[_0-9A-Za-z]*]] = OpTypeFloat 32
// CHECK: [[V4FLOAT:%[_0-9A-Za-z]*]] = OpTypeVector [[FLOAT]] 4
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
      float4 vTest = g_PushConstants.m_nBufferDeviceAddress.Get().g_vTestFloat4;
      float f = vk::BufferPointer<float,4>(0xdeadbeefull).Get();
      return vTest+f;
}

// CHECK: [[FUN]] = OpFunction
// CHECK: [[X1:%[_0-9A-Za-z]*]] = OpAccessChain [[PPGLOBALS]] [[GPC]] [[S0]]
// CHECK: [[X2:%[_0-9A-Za-z]*]] = OpLoad [[PGLOBALS]] [[X1]]
// CHECK: [[X3:%[_0-9A-Za-z]*]] = OpAccessChain [[PV4FLOAT2]] [[X2]] [[S1]]
// CHECK: [[X4:%[_0-9A-Za-z]*]] = OpLoad [[V4FLOAT]] [[X3]] Aligned 16
// CHECK: [[TEMP_PTR:%[_0-9A-Za-z]*]] = OpConvertUToPtr %_ptr_PhysicalStorageBuffer_float %ulong_3735928559
// CHECK: [[LD:%[_0-9A-Za-z]*]] = OpLoad %float [[TEMP_PTR]] Aligned 4
// CHECK: [[CONSTRUCT:%[_0-9A-Za-z]*]] = OpCompositeConstruct [[V4FLOAT]] [[LD]] [[LD]] [[LD]] [[LD]]
// CHECK: [[ADD:%[_0-9A-Za-z]*]] = OpFAdd [[V4FLOAT]] [[X4]] [[CONSTRUCT]]
// CHECK: OpStore [[OUT]] [[ADD]]
// CHECK: OpFunctionEnd
