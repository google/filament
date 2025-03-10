#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T>
struct spvBufferDescriptor
{
    T value;
    int length;
    const device T& operator -> () const device
    {
        return value;
    }
    const device T& operator * () const device
    {
        return value;
    }
};

template<typename T>
struct spvDescriptorArray;

template<typename T>
struct spvDescriptorArray<device T*>
{
    spvDescriptorArray(const device spvBufferDescriptor<device T*>* ptr) : ptr(ptr)
    {
    }
    const device T* operator [] (size_t i) const
    {
        return ptr[i].value;
    }
    const int length(int i) const
    {
        return ptr[i].length;
    }
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

fragment void main0(main0_in in [[stage_in]], const device spvBufferDescriptor<const device Ssbo*>* ssbo_ [[buffer(0)]])
{
    spvDescriptorArray<const device Ssbo*> ssbo {ssbo_};

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

