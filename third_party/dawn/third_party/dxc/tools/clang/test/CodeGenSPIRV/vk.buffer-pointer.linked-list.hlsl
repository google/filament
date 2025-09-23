// RUN: %dxc -spirv -Od -T ps_6_0 -E MainPs %s | FileCheck %s

// CHECK: OpCapability PhysicalStorageBufferAddresses
// CHECK: OpExtension "SPV_KHR_physical_storage_buffer"
// CHECK: OpMemoryModel PhysicalStorageBuffer64 GLSL450
// CHECK: OpEntryPoint Fragment [[MAIN:%[_0-9A-Za-z]*]] "MainPs" [[OUT:%[_0-9A-Za-z]*]]

// Forward declaration
typedef struct block_s block_t;
typedef vk::BufferPointer<block_t, 32> block_p;

struct block_s
{
      float4 x;
      block_p next;
};

struct TestPushConstant_t
{
      block_p root;
};

[[vk::push_constant]] TestPushConstant_t g_PushConstants;

// CHECK: OpDecorate [[GP:%[_0-9A-Za-z]*]] AliasedPointer
// CHECK: OpDecorate [[COPY1:%[_0-9A-Za-z]*]] RestrictPointer
// CHECK: OpDecorate [[COPY2:%[_0-9A-Za-z]*]] RestrictPointer
// CHECK: OpMemberDecorate [[BLOCK:%[_0-9A-Za-z]*]] 1 Offset 16
// CHECK: OpTypeForwardPointer [[PBLOCK:%[_0-9A-Za-z]*]] PhysicalStorageBuffer
// CHECK: [[SINT:%[_0-9A-Za-z]*]] = OpTypeInt 32 1
// CHECK-DAG: [[S0:%[_0-9A-Za-z]*]] = OpConstant [[SINT]] 0
// CHECK-DAG: [[S1:%[_0-9A-Za-z]*]] = OpConstant [[SINT]] 1
// CHECK: [[ULONG:%[_0-9A-Za-z]*]] = OpTypeInt 64 0
// CHECK: [[UL0:%[_0-9A-Za-z]*]] = OpConstant [[ULONG]] 0
// CHECK: [[FLOAT:%[_0-9A-Za-z]*]] = OpTypeFloat 32
// CHECK: [[F0:%[_0-9A-Za-z]*]] = OpConstant [[FLOAT]] 0
// CHECK: [[V4FLOAT:%[_0-9A-Za-z]*]] = OpTypeVector [[FLOAT]] 4
// CHECK: [[CV4FLOAT:%[_0-9A-Za-z]*]] = OpConstantComposite [[V4FLOAT]] [[F0]] [[F0]] [[F0]] [[F0]]
// CHECK: [[BLOCK]] = OpTypeStruct [[V4FLOAT]] [[PBLOCK]]
// CHECK: [[PBLOCK]] = OpTypePointer PhysicalStorageBuffer [[BLOCK]]
// CHECK: [[PC:%[_0-9A-Za-z]*]] = OpTypeStruct [[PBLOCK]]
// CHECK: [[PPC:%[_0-9A-Za-z]*]] = OpTypePointer PushConstant [[PC]]
// CHECK: [[PV4FLOAT1:%[_0-9A-Za-z]*]] = OpTypePointer Output [[V4FLOAT]]
// CHECK: [[PPBLOCK0:%[_0-9A-Za-z]*]] = OpTypePointer Function %_ptr_PhysicalStorageBuffer_block_s
// CHECK: [[PPBLOCK1:%[_0-9A-Za-z]*]] = OpTypePointer PushConstant [[PBLOCK]]
// CHECK: [[PPBLOCK2:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[PBLOCK]]
// CHECK: [[BOOL:%[_0-9A-Za-z]*]] = OpTypeBool
// CHECK: [[PV4FLOAT2:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[V4FLOAT]]
// CHECK: [[GPC:%[_0-9A-Za-z]*]] = OpVariable [[PPC]] PushConstant
// CHECK: [[OUT]] = OpVariable [[PV4FLOAT1]] Output

[numthreads(1,1,1)]
float4 MainPs(void) : SV_Target0
{
  if (__has_feature(hlsl_vk_buffer_pointer)) {
      [[vk::aliased_pointer]] block_p g_p =
          vk::static_pointer_cast<block_t, 16>(g_PushConstants.root);
      g_p = g_p.Get().next;
      uint64_t addr = (uint64_t)g_p;
      block_p copy1 = block_p(addr);
      block_p copy2 = block_p(copy1);
      if (addr == 0) // Null pointer test
          return float4(0.0,0.0,0.0,0.0);
      return g_p.Get().x;
  }
  return float4(0.0,0.0,0.0,0.0);
}

// CHECK: [[MAIN]] = OpFunction
// CHECK-NEXT: OpLabel
// CHECK-NEXT: [[RESULT:%[_0-9A-Za-z]*]] = OpFunctionCall [[V4FLOAT]] [[FUN:%[_0-9A-Za-z]*]]
// CHECK: OpStore [[OUT]] [[RESULT]]
// CHECK: OpFunctionEnd
// CHECK: [[FUN]] = OpFunction [[V4FLOAT]]
// CHECK: [[GP]] = OpVariable [[PPBLOCK0]] Function
// CHECK: [[X1:%[_0-9A-Za-z]*]] = OpAccessChain [[PPBLOCK1]] [[GPC]] [[S0]]
// CHECK: [[X2:%[_0-9A-Za-z]*]] = OpLoad [[PBLOCK]] [[X1]]
// CHECK: OpStore [[GP]] [[X2]]
// CHECK: [[X3:%[_0-9A-Za-z]*]] = OpLoad [[PBLOCK]] [[GP]]
// CHECK: [[X4:%[_0-9A-Za-z]*]] = OpAccessChain [[PPBLOCK2]] [[X3]] [[S1]]
// CHECK: [[X5:%[_0-9A-Za-z]*]] = OpLoad [[PBLOCK]] [[X4]] Aligned 32
// CHECK: OpStore [[GP]] [[X5]]
// CHECK: [[X6:%[_0-9A-Za-z]*]] = OpLoad [[PBLOCK]] [[GP]]
// CHECK: [[X7:%[_0-9A-Za-z]*]] = OpConvertPtrToU [[ULONG]] [[X6]]
// CHECK: OpStore [[ADDR:%[_0-9A-Za-z]*]] [[X7]]
// CHECK: [[X8:%[_0-9A-Za-z]*]] = OpLoad [[ULONG]] [[ADDR]]
// CHECK: [[X9:%[_0-9A-Za-z]*]] = OpConvertUToPtr [[PBLOCK]] [[X8]]
// CHECK: OpStore [[COPY1]] [[X9]]
// CHECK: [[X10:%[_0-9A-Za-z]*]] = OpLoad [[PBLOCK]] [[COPY1]]
// CHECK: OpStore [[COPY2]] [[X10]]
// CHECK: [[X11:%[_0-9A-Za-z]*]] = OpLoad [[ULONG]] [[ADDR]]
// CHECK: [[X12:%[_0-9A-Za-z]*]] = OpIEqual %bool [[X11]] [[UL0]]
// CHECK: OpBranchConditional [[X12]] [[IF_TRUE:%[_0-9A-Za-z]*]] [[IF_MERGE:%[_0-9A-Za-z]*]]
// CHECK: [[IF_TRUE]] = OpLabel
// CHECK: OpReturnValue [[CV4FLOAT]]
// CHECK: [[IF_MERGE]] = OpLabel
// CHECK: [[X13:%[_0-9A-Za-z]*]] = OpLoad [[PBLOCK]] [[GP]]
// CHECK: [[X14:%[_0-9A-Za-z]*]] = OpAccessChain [[PV4FLOAT2]] [[X13]] [[S0]]
// CHECK: [[X15:%[_0-9A-Za-z]*]] = OpLoad [[V4FLOAT]] [[X14]] Aligned 16
// CHECK: OpReturnValue [[X15]]
// CHECK: OpFunctionEnd
