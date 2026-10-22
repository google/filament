// RUN: %dxc -spirv -E main -T cs_6_7 %s -fspv-target-env=vulkan1.3 | FileCheck %s

template <typename T>
struct BufferAccessor {
    vk::BufferPointer<T> ptr;
};

cbuffer Test {
    vk::BufferPointer<float> buffer;
};

[numthreads(256, 1, 1)]
void main(in uint3 threadId : SV_DispatchThreadID) {
// CHECK: [[AC:%[_0-9A-Za-z]*]] = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_float %Test %int_0
// CHECK: [[PTR:%[_0-9A-Za-z]*]] = OpLoad %_ptr_PhysicalStorageBuffer_float [[AC]]
    BufferAccessor<float> accessor = BufferAccessor<float>(buffer);

// CHECK: [[VAL:%[_0-9A-Za-z]*]] = OpLoad %float [[PTR]] Aligned 4
// CHECK: OpStore [[PTR]] [[VAL]] Aligned 4
    buffer.Get() = accessor.ptr.Get();
}
