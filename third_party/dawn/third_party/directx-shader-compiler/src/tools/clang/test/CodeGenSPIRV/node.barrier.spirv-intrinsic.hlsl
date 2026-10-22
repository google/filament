// RUN: %dxc -T lib_6_8 -spirv -fspv-target-env=vulkan1.3 -HV 202x -fvk-use-scalar-layout -E main %s | FileCheck %s

#define MemorySemanticsAcquireReleaseMask 0x00000008
#define MemorySemanticsWorkgroupMemoryMask 0x00000100
#define ScopeWorkgroup 2
#define OpControlBarrier 224

[[vk::ext_instruction( OpControlBarrier )]]
void controlBarrier(uint32_t executionScope, uint32_t memoryScope, uint32_t memorySemantics);

[numthreads(64,1,1)]
[shader("compute")]
void main(uint32_t3 ID : SV_DispatchThreadID)
{
  controlBarrier(ScopeWorkgroup, ScopeWorkgroup, MemorySemanticsAcquireReleaseMask | MemorySemanticsWorkgroupMemoryMask);
}

// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U2:%[^ ]*]] = OpConstant [[UINT]] 2
// CHECK-DAG: [[U264:%[^ ]*]] = OpConstant [[UINT]] 264
// CHECK: OpControlBarrier [[U2]] [[U2]] [[U264]]

