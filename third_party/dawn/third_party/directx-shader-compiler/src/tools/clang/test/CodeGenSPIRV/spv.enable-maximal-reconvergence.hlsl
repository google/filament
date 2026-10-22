// RUN: %dxc -T cs_6_0 -E main -fspv-enable-maximal-reconvergence %s -spirv | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-ENABLED
// RUN: %dxc -T cs_6_0 -E main %s -spirv | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-DISABLED

// CHECK-ENABLED:            OpExtension "SPV_KHR_maximal_reconvergence"
// CHECK-DISABLED-NOT:       OpExtension "SPV_KHR_maximal_reconvergence"
// CHECK:                    OpEntryPoint GLCompute %main "main"
// CHECK-ENABLED:            OpExecutionMode %main MaximallyReconvergesKHR
// CHECK-DISABLED-NOT:       OpExecutionMode %main MaximallyReconvergesKHR
[numthreads(1, 1, 1)]
void main() {}

