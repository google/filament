#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>
#if __METAL_VERSION__ >= 230
#include <metal_raytracing>
using namespace metal::raytracing;
#endif

using namespace metal;

intersection_params spvMakeIntersectionParams(uint flags)
{
    intersection_params ip;
    if ((flags & 1) != 0)
        ip.force_opacity(forced_opacity::opaque);
    if ((flags & 2) != 0)
        ip.force_opacity(forced_opacity::non_opaque);
    if ((flags & 4) != 0)
        ip.accept_any_intersection(true);
    if ((flags & 16) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::back);
    if ((flags & 32) != 0)
        ip.set_triangle_cull_mode(triangle_cull_mode::front);
    if ((flags & 64) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::opaque);
    if ((flags & 128) != 0)
        ip.set_opacity_cull_mode(opacity_cull_mode::non_opaque);
    if ((flags & 256) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::triangle);
    if ((flags & 512) != 0)
        ip.set_geometry_cull_mode(geometry_cull_mode::bounding_box);
    return ip;
}

template<typename T>
struct spvDescriptor
{
    T value;
};

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
struct spvDescriptorArray
{
    spvDescriptorArray(const device spvDescriptor<T>* ptr_) : ptr(&ptr_->value) {}
    spvDescriptorArray(const device void *ptr_) : spvDescriptorArray(static_cast<const device spvDescriptor<T>*>(ptr_)) {}
    const device T& operator [] (size_t i) const { return ptr[i]; }
    const device T* ptr;
};

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

struct Ubo
{
    uint val;
};

struct main0_in
{
    uint inputId [[user(locn0)]];
};

fragment void main0(main0_in in [[stage_in]], device const void* spvDescriptorSet0Binding3 [[buffer(4)]], device const void* spvDescriptorSet0Binding4 [[buffer(5)]], device const void* spvDescriptorSet0Binding0 [[buffer(0)]], device const void* spvDescriptorSet0Binding2 [[buffer(2)]], device const void* spvDescriptorSet0Binding5 [[buffer(6)]], device const void* spvDescriptorSet0Binding0Smplr [[buffer(1)]], device const void* spvDescriptorSet0Binding1 [[buffer(3)]], device const void* spvDescriptorSet0Binding6 [[buffer(7)]])
{
    spvDescriptorArray<texture2d<float>> smp_textures {spvDescriptorSet0Binding0};
    spvDescriptorArray<sampler> smp_texturesSmplr {spvDescriptorSet0Binding0Smplr};
    spvDescriptorArray<texture2d<float>> textures {spvDescriptorSet0Binding2};
    spvDescriptorArray<sampler> smp {spvDescriptorSet0Binding1};
    spvDescriptorArray<const device Ssbo*> ssbo {spvDescriptorSet0Binding3};
    spvDescriptorArray<constant Ubo*> ubo {spvDescriptorSet0Binding4};
    spvDescriptorArray<texture2d<float>> images {spvDescriptorSet0Binding5};
    spvDescriptorArray<raytracing::acceleration_structure<raytracing::instancing>> tlas {spvDescriptorSet0Binding6};

    uint _231 = in.inputId;
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery;
    raytracing::intersection_query<raytracing::instancing, raytracing::triangle_data> rayQuery_1;
    if (smp_textures[_231].sample(smp_texturesSmplr[_231], float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    uint _249 = in.inputId + 8u;
    if (textures[_231].sample(smp[_249], float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    if (ssbo[_231]->val == 2u)
    {
        discard_fragment();
    }
    if (int((ssbo.length(123) - 4) / 4) == 25)
    {
        discard_fragment();
    }
    if (ubo[_231]->val == 2u)
    {
        discard_fragment();
    }
    if (images[_231].read(uint2(int2(0))).w > 0.5)
    {
        discard_fragment();
    }
    rayQuery.reset(ray(float3(0.0), float3(1.0), 0.00999999977648258209228515625, 1.0), tlas[in.inputId], 255u, spvMakeIntersectionParams(0u));
    bool _301 = rayQuery.next();
    if (smp_textures[_231].sample(smp_texturesSmplr[_231], float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    if (textures[_231].sample(smp[_231], float2(0.0), level(0.0)).w > 0.5)
    {
        discard_fragment();
    }
    if (images[_231].read(uint2(int2(0))).w > 0.5)
    {
        discard_fragment();
    }
    rayQuery_1.reset(ray(float3(0.0), float3(1.0), 0.00999999977648258209228515625, 1.0), tlas[in.inputId], 255u, spvMakeIntersectionParams(0u));
    bool _336 = rayQuery_1.next();
}

