// RUN: %dxc -T cs_6_6 -spirv -fspv-target-env=vulkan1.3 %s | FileCheck %s --check-prefix=GOOD
// RUN: not %dxc -T cs_6_6 -spirv -fspv-target-env=vulkan1.3 -DBAD %s 2>&1 | FileCheck %s --check-prefix=BAD

struct Base {
};

struct Derived : Base {
    int val;
};

cbuffer Test {
    vk::BufferPointer<Derived, 32> derivedBuf;
    vk::BufferPointer<int> intBuf;
};

[shader("compute")]
[numthreads(256, 1, 1)]
void main(in uint3 threadId : SV_DispatchThreadID) {
#ifdef BAD
    vk::BufferPointer<Base, 64> derivedBufAsBase = vk::static_pointer_cast<Base, 64>(derivedBuf);
    vk::BufferPointer<float> intBufAsFloat = vk::static_pointer_cast<float>(intBuf);
#else
    vk::BufferPointer<Base, 16> derivedBufAsBase = vk::static_pointer_cast<Base, 16>(derivedBuf);
    vk::BufferPointer<float> intBufAsFloat = vk::reinterpret_pointer_cast<float>(intBuf);
#endif

    intBuf.Get() = (int)intBufAsFloat.Get();
}

// GOOD: [[INT:%[^ ]*]] = OpTypeInt 32 1
// GOOD: [[I1:%[^ ]*]] = OpConstant [[INT]] 1
// GOOD: [[PINT:%[^ ]*]] = OpTypePointer PhysicalStorageBuffer [[INT]]
// GOOD: [[FLOAT:%[^ ]*]] = OpTypeFloat 32
// GOOD: [[PFLOAT:%[^ ]*]] = OpTypePointer PhysicalStorageBuffer [[FLOAT]]
// GOOD: %Test = OpVariable %{{[^ ]*}} Uniform
// GOOD: [[V0:%[^ ]*]] = OpAccessChain %{{[^ ]*}} %Test [[I1]]
// GOOD: [[V1:%[^ ]*]] = OpLoad [[PINT]] [[V0]]
// GOOD: [[V2:%[^ ]*]] = OpBitcast [[PFLOAT]] [[V1]]
// GOOD: [[V3:%[^ ]*]] = OpLoad [[FLOAT]] [[V2]] Aligned 4
// GOOD: [[V4:%[^ ]*]] = OpConvertFToS [[INT]] [[V3]]
// GOOD: OpStore [[V1]] [[V4]] Aligned 4

// BAD: error: Vulkan buffer pointer cannot be cast to greater alignment
// BAD: error: vk::static_pointer_cast() content type must be base class of argument's content type

