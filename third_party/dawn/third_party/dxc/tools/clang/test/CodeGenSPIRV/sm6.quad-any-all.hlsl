// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fcgl  %s -spirv | FileCheck %s --check-prefixes=CHECK,QUAD
// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1 -fspv-extension=SPV_KHR_16bit_storage -fcgl  %s -spirv | FileCheck %s --check-prefixes=CHECK,NOQUAD
// RUN: not %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.0 -fcgl  %s -spirv 2>&1 | FileCheck %s --check-prefixes=ERROR

// CHECK: ; Version: 1.3

// QUAD: OpCapability QuadControlKHR
// QUAD: OpExtension "SPV_KHR_quad_control"

RWStructuredBuffer<float3> values;

[numthreads(32, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
  uint outIdx = (id.y * 8) + id.x;

// CHECK:        [[val1:%[0-9]+]] = OpIEqual %bool {{%[0-9]+}}
// QUAD-NEXT:         {{%[0-9]+}} = OpGroupNonUniformQuadAnyKHR %bool [[val1]]

// NOQUAD-NEXT: [[inv0:%[0-9]+]] = OpGroupNonUniformQuadSwap %bool %uint_3 [[val1]] %uint_0
// NOQUAD-NEXT:  [[or0:%[0-9]+]] = OpLogicalOr %bool [[val1]] [[inv0]]
// NOQUAD-NEXT: [[inv1:%[0-9]+]] = OpGroupNonUniformQuadSwap %bool %uint_3 [[val1]] %uint_1
// NOQUAD-NEXT:  [[or1:%[0-9]+]] = OpLogicalOr %bool [[or0]] [[inv1]]
// NOQUAD-NEXT: [[inv2:%[0-9]+]] = OpGroupNonUniformQuadSwap %bool %uint_3 [[val1]] %uint_2
// NOQUAD-NEXT:  [[or2:%[0-9]+]] = OpLogicalOr %bool [[or1]] [[inv2]]

// ERROR: 27:24: error: Vulkan 1.1 is required for Wave Operation but not permitted to use
    values[outIdx].x = QuadAny(outIdx % 4 == 0) ? 1.0 : 2.0;

// CHECK:        [[val2:%[0-9]+]] = OpIEqual %bool {{%[0-9]+}}
// QUAD-NEXT:         {{%[0-9]+}} = OpGroupNonUniformQuadAllKHR %bool [[val2]]

// NOQUAD-NEXT: [[inv0:%[0-9]+]] = OpGroupNonUniformQuadSwap %bool %uint_3 [[val2]] %uint_0
// NOQUAD-NEXT:  [[or0:%[0-9]+]] = OpLogicalAnd %bool [[val2]] [[inv0]]
// NOQUAD-NEXT: [[inv1:%[0-9]+]] = OpGroupNonUniformQuadSwap %bool %uint_3 [[val2]] %uint_1
// NOQUAD-NEXT:  [[or1:%[0-9]+]] = OpLogicalAnd %bool [[or0]] [[inv1]]
// NOQUAD-NEXT: [[inv2:%[0-9]+]] = OpGroupNonUniformQuadSwap %bool %uint_3 [[val2]] %uint_2
// NOQUAD-NEXT:  [[or2:%[0-9]+]] = OpLogicalAnd %bool [[or1]] [[inv2]]

// ERROR: 40:24: error: Vulkan 1.1 is required for Wave Operation but not permitted to use
    values[outIdx].y = QuadAll(outIdx % 2 == 0) ? 3.0 : 4.0;
}
