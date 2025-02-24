#version 450 core

#extension GL_ARB_gpu_shader_int64: enable
#extension GL_EXT_buffer_reference: enable
#extension GL_EXT_spirv_intrinsics: enable

#define CapabilityPhysicalStorageBufferAddresses   5347
#define StorageClassPhysicalStorageBuffer          5349
#define OpTypePointer                              32
#define OpLoad                                     61
#define OpConvertUToPtr                            120

#define uintStoragePtr spirv_type(extensions = ["SPV_EXT_physical_storage_buffer", "SPV_KHR_variable_pointers"], capabilities = [CapabilityPhysicalStorageBufferAddresses], id = OpTypePointer, StorageClassPhysicalStorageBuffer, uint)

// Just to enable the memory model of physical storage buffer
layout(buffer_reference, std430) buffer Dummy {
  uint dummy;
};

spirv_instruction(id = OpLoad) uint loadUint(uintStoragePtr pointer, spirv_literal uint memoryOperands, spirv_literal uint alignment);
spirv_instruction(id = OpConvertUToPtr) uintStoragePtr convertToPtr(uint64_t value);

void main() {
  uint value = loadUint(convertToPtr(1), 0x2, 32);
}
