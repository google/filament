// RUN: not %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.1spirv1.3 %s -spirv 2>&1 | FileCheck %s

[numthreads(1, 1, 1)]
void main() { }

// CHECK: error: unknown SPIR-V target environment 'vulkan1.1spirv1.3'
