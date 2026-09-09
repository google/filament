// RUN: %dxc -spirv -E main -T cs_6_7 %s | FileCheck %s


struct Foo {
    int val[4];
};

// CHECK: OpDecorate [[ARRAY_INT_4:%[_0-9A-Za-z]*]] ArrayStride 4
// CHECK: [[TYPE_FOO:%[_0-9A-Za-z]*]] = OpTypeStruct [[ARRAY_INT_4]]
// CHECK: [[PTR_PHYS_BUF_FOO:%[_0-9A-Za-z]*]] = OpTypePointer PhysicalStorageBuffer [[TYPE_FOO]]
// CHECK: [[TYPE_TEST_STRUCT:%[_0-9A-Za-z]*]] = OpTypeStruct [[PTR_PHYS_BUF_FOO]] %_ptr_PhysicalStorageBuffer_int
// CHECK: [[PTR_UNIFORM_TYPE_TEST:%[_0-9A-Za-z]*]] = OpTypePointer Uniform [[TYPE_TEST_STRUCT]]
// CHECK: [[TEST_VAR:%[_0-9A-Za-z]*]] = OpVariable [[PTR_UNIFORM_TYPE_TEST]] Uniform
cbuffer Test {
    // The layout of `Foo` for the Buffer pointer should be the layout for
    // the storage buffer, even if the pointer is in a cbuffer.
    vk::BufferPointer<Foo> fooBuf;
    vk::BufferPointer<int> outBuf;
};



[numthreads(256, 1, 1)]
void main(in uint3 threadId : SV_DispatchThreadID) {
// CHECK: [[FOO_BUF_ACCESS_CHAIN:%[_0-9A-Za-z]*]] = OpAccessChain %_ptr_Uniform__ptr_PhysicalStorageBuffer_Foo [[TEST_VAR]] %int_0
// CHECK: [[FOO_BUF_LOAD:%[_0-9A-Za-z]*]] = OpLoad %_ptr_PhysicalStorageBuffer_Foo [[FOO_BUF_ACCESS_CHAIN]]
    int val = fooBuf.Get().val[0];
    outBuf.Get() = val;
}
