// RUN: %dxc -E main -T ps_6_1 -fspv-target-env=vulkan1.1 -Zi -fcgl  %s -spirv | FileCheck %s

// This test ensures that command line options used to generate this module
// are added to the SPIR-V using OpModuleProcessed.

// CHECK: OpModuleProcessed "dxc-cl-option:
// CHECK-SAME: -E main -T ps_6_1 
// CHECK-SAME: -fspv-target-env=vulkan1.1 -Zi

void main() {}
