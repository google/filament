#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T>
struct spvBufferDescriptor;

template<typename T>
struct spvBufferDescriptor<device T*>
{
    device T* value;
    int length;
    int padding;
};

template<typename T>
struct spvDescriptorArray;

template<typename T>
struct spvDescriptorArray<device T*>
{
    spvDescriptorArray(const device spvBufferDescriptor<device T*>* ptr_) : ptr(ptr_) {}
    spvDescriptorArray(const device void *ptr_) : spvDescriptorArray(static_cast<const device spvBufferDescriptor<device T*>*>(ptr_)) {}
    device T* operator [] (size_t i) const { return ptr[i].value; }
    int length(int i) const { return ptr[i].length; }
    const device spvBufferDescriptor<device T*>* ptr;
};

struct Ssbo
{
    uint val;
    uint data[1];
};

struct main0_in
{
    uint inputId [[user(locn0)]];
};

fragment void main0(main0_in in [[stage_in]], device const void* spvDescriptorSet0Binding0 [[buffer(0)]])
{
    spvDescriptorArray<const device Ssbo*> ssbo {spvDescriptorSet0Binding0};

    uint _15 = in.inputId;
    if (ssbo[_15]->val == 2u)
    {
        discard_fragment();
    }
    if (int((ssbo.length(123) - 4) / 4) == 25)
    {
        discard_fragment();
    }
}

