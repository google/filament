// RUN: %dxc -T cs_6_6 -E main -fcgl -spirv %s 2>&1 | FileCheck %s

// CHECK: warning: Wave size is not supported by Vulkan SPIR-V. Consider using VK_EXT_subgroup_size_control.

[WaveSize(32)]
[numthreads(1, 1, 1)]
void main() {}