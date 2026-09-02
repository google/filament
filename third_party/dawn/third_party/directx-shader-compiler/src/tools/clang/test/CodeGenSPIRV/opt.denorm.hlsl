// RUN: %dxc -E main -T ps_6_2 -denorm preserve -spirv %s | FileCheck --check-prefixes=CHECK-PRE %s
// RUN: %dxc -E main -T ps_6_2 -denorm preserve -spirv -fspv-target-env=vulkan1.2 %s | FileCheck --check-prefixes=CHECK-PRE-VK12 %s
// RUN: %dxc -E main -T ps_6_2 -denorm ftz -spirv %s | FileCheck --check-prefixes=CHECK-FTZ %s
// RUN: %dxc -E main -T ps_6_2 -denorm any -spirv %s | FileCheck --check-prefixes=CHECK-DEFAULT %s
// RUN: %dxc -E main -T ps_6_2 -spirv %s | FileCheck --check-prefixes=CHECK-DEFAULT %s

// CHECK-PRE: OpCapability DenormPreserve
// CHECK-PRE: OpExtension "SPV_KHR_float_controls"
// CHECK-PRE: OpExecutionMode %main DenormPreserve 32

// CHECK-PRE-VK12:     OpCapability DenormPreserve
// CHECK-PRE-VK12-NOT: OpExtension "SPV_KHR_float_controls"
// CHECK-PRE-VK12:     OpExecutionMode %main DenormPreserve 32

// CHECK-FTZ: OpCapability DenormFlushToZero
// CHECK-FTZ: OpExtension "SPV_KHR_float_controls"
// CHECK-FTZ: OpExecutionMode %main DenormFlushToZero 32

// CHECK-DEFAULT-NOT: OpCapability DenormPreserve
// CHECK-DEFAULT-NOT: OpExtension "SPV_KHR_float_controls"
// CHECK-DEFAULT-NOT: OpExecutionMode %main Denorm
float4 main(float4 col : COL) : SV_Target {
    return col;
}
