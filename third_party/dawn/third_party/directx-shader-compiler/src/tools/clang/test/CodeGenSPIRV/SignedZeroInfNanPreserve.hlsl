// RUN: %dxc -T cs_6_0 -spirv -Gis %s| FileCheck %s --check-prefixes=CHECK,PRE_1_2
// RUN: %dxc -T cs_6_0 -spirv -Gis -fspv-target-env=vulkan1.2 %s| FileCheck %s 

// CHECK: OpCapability SignedZeroInfNanPreserve
// PRE_1_2: OpExtension "SPV_KHR_float_controls"
// CHECK: OpEntryPoint
// CHECK: OpExecutionMode %main SignedZeroInfNanPreserve 32

[numthreads(8,1,1)]
void main() {}
