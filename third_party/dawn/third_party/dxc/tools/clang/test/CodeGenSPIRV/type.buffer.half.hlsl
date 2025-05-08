// RUN: not %dxc -T ps_6_6 -E main -fcgl  %s -spirv -enable-16bit-types  2>&1 | FileCheck %s --check-prefix=VK
// RUN: %dxc -T ps_6_6 -E main -fcgl  %s -spirv -fspv-target-env=universal1.5 -enable-16bit-types  2>&1 | FileCheck %s --check-prefix=UNIVERSAL

// When targeting Vulkan, A 16-bit floating pointer buffer is not valid.
// VK: error: The sampled type for textures cannot be a floating point type smaller than 32-bits when targeting a Vulkan environment.

// When not targeting Vulkan, we should generate the 16-bit floating pointer buffer.
// UNIVERSAL: %half = OpTypeFloat 16
// UNIVERSAL: %type_buffer_image = OpTypeImage %half Buffer 2 0 0 1 Unknown
// UNIVERSAL: %_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
// UNIVERSAL: %MyBuffer = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
Buffer<half> MyBuffer;

void main(): SV_Target { }
