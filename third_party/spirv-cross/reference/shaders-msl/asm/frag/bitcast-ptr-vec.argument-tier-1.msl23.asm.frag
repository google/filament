#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T>
struct spvDescriptor
{
    T value;
};

template<typename T>
struct spvDescriptorArray
{
    spvDescriptorArray(const device spvDescriptor<T>* ptr) : ptr(&ptr->value)
    {
    }
    const device T& operator [] (size_t i) const
    {
        return ptr[i];
    }
    const device T* ptr;
};

struct type_ConstantBuffer_PushConstants
{
    ulong VertexShaderConstants;
    ulong PixelShaderConstants;
    ulong SharedConstants;
};

struct main0_out
{
    float4 out_var_SV_Target [[color(0)]];
};

fragment main0_out main0(constant type_ConstantBuffer_PushConstants& g_PushConstants [[buffer(0)]], const device spvDescriptor<texture2d<float>>* g_Texture2DDescriptorHeap_ [[buffer(1)]], float4 gl_FragCoord [[position]])
{
    spvDescriptorArray<texture2d<float>> g_Texture2DDescriptorHeap {g_Texture2DDescriptorHeap_};

    main0_out out = {};
    int2 _55 = int2(gl_FragCoord.xy) - (*(reinterpret_cast<device int2*>(g_PushConstants.SharedConstants + 16ul)));
    bool _66;
    if (!any(_55 < int2(0)))
    {
        _66 = any(_55 >= (*(reinterpret_cast<device int2*>(g_PushConstants.SharedConstants + 24ul))));
    }
    else
    {
        _66 = true;
    }
    float4 _77;
    if (_66)
    {
        _77 = float4(0.0);
    }
    else
    {
        _77 = g_Texture2DDescriptorHeap[*(reinterpret_cast<device uint*>(g_PushConstants.SharedConstants + 12ul))].read(uint2(int3(select(_55, int2(0), bool2(_66)), 0).xy), 0);
    }
    float3 _81 = powr(_77.xyz, *(reinterpret_cast<device float3*>(g_PushConstants.SharedConstants)));
    out.out_var_SV_Target = float4(_81.x, _81.y, _81.z, _77.w);
    return out;
}

