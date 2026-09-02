// RUN: %dxc -T vs_6_5 -P -spirv -fspv-target-env=vulkan1.0 -Fi %t.vk_1_0.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vk_1_0.hlsl.pp %s --check-prefix=VK_1_0
// VK_1_0: SPIR-V version 1 0

// RUN: %dxc -T vs_6_5 -P -spirv -fspv-target-env=vulkan1.1 -Fi %t.vk_1_1.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vk_1_1.hlsl.pp %s --check-prefix=VK_1_1
// VK_1_1: SPIR-V version 1 3

// RUN: %dxc -T vs_6_5 -P -spirv -fspv-target-env=vulkan1.1spirv1.4 -Fi %t.vk_1_1.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vk_1_1.hlsl.pp %s --check-prefix=VK_1_1_SPV_1_4
// VK_1_1_SPV_1_4: SPIR-V version 1 4

// RUN: %dxc -T vs_6_5 -P -spirv -fspv-target-env=vulkan1.2 -Fi %t.vk_1_2.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vk_1_2.hlsl.pp %s --check-prefix=VK_1_2
// VK_1_2: SPIR-V version 1 5

// RUN: %dxc -T vs_6_5 -P -spirv -fspv-target-env=universal1.5 -Fi %t.universal_1_5.hlsl.pp %s
// RUN: FileCheck --input-file=%t.universal_1_5.hlsl.pp %s --check-prefix=UNI_1_5
// UNI_1_5: SPIR-V version 1 5

// RUN: %dxc -T vs_6_5 -P -spirv -fspv-target-env=vulkan1.3 -Fi %t.vk_1_3.hlsl.pp %s
// RUN: FileCheck --input-file=%t.vk_1_3.hlsl.pp %s --check-prefix=VK_1_3
// VK_1_3: SPIR-V version 1 6

SPIR-V version __SPIRV_MAJOR_VERSION__ __SPIRV_MINOR_VERSION__
