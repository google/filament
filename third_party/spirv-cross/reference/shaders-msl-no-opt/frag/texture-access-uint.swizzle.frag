#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Returns 2D texture coords corresponding to 1D texel buffer coords
static inline __attribute__((always_inline))
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

template<typename T> struct spvRemoveReference { typedef T type; };
template<typename T> struct spvRemoveReference<thread T&> { typedef T type; };
template<typename T> struct spvRemoveReference<thread T&&> { typedef T type; };
template<typename T> inline constexpr thread T&& spvForward(thread typename spvRemoveReference<T>::type& x)
{
    return static_cast<thread T&&>(x);
}
template<typename T> inline constexpr thread T&& spvForward(thread typename spvRemoveReference<T>::type&& x)
{
    return static_cast<thread T&&>(x);
}

enum class spvSwizzle : uint
{
    none = 0,
    zero,
    one,
    red,
    green,
    blue,
    alpha
};

template<typename T>
inline T spvGetSwizzle(vec<T, 4> x, T c, spvSwizzle s)
{
    switch (s)
    {
        case spvSwizzle::none:
            return c;
        case spvSwizzle::zero:
            return 0;
        case spvSwizzle::one:
            return 1;
        case spvSwizzle::red:
            return x.r;
        case spvSwizzle::green:
            return x.g;
        case spvSwizzle::blue:
            return x.b;
        case spvSwizzle::alpha:
            return x.a;
    }
}

// Wrapper function that swizzles texture samples and fetches.
template<typename T>
inline vec<T, 4> spvTextureSwizzle(vec<T, 4> x, uint s)
{
    if (!s)
        return x;
    return vec<T, 4>(spvGetSwizzle(x, x.r, spvSwizzle((s >> 0) & 0xFF)), spvGetSwizzle(x, x.g, spvSwizzle((s >> 8) & 0xFF)), spvGetSwizzle(x, x.b, spvSwizzle((s >> 16) & 0xFF)), spvGetSwizzle(x, x.a, spvSwizzle((s >> 24) & 0xFF)));
}

template<typename T>
inline T spvTextureSwizzle(T x, uint s)
{
    return spvTextureSwizzle(vec<T, 4>(x, 0, 0, 1), s).x;
}

// Wrapper function that swizzles texture gathers.
template<typename T, template<typename, access = access::sample, typename = void> class Tex, typename... Ts>
inline vec<T, 4> spvGatherSwizzle(const thread Tex<T>& t, sampler s, uint sw, component c, Ts... params) METAL_CONST_ARG(c)
{
    if (sw)
    {
        switch (spvSwizzle((sw >> (uint(c) * 8)) & 0xFF))
        {
            case spvSwizzle::none:
                break;
            case spvSwizzle::zero:
                return vec<T, 4>(0, 0, 0, 0);
            case spvSwizzle::one:
                return vec<T, 4>(1, 1, 1, 1);
            case spvSwizzle::red:
                return t.gather(s, spvForward<Ts>(params)..., component::x);
            case spvSwizzle::green:
                return t.gather(s, spvForward<Ts>(params)..., component::y);
            case spvSwizzle::blue:
                return t.gather(s, spvForward<Ts>(params)..., component::z);
            case spvSwizzle::alpha:
                return t.gather(s, spvForward<Ts>(params)..., component::w);
        }
    }
    switch (c)
    {
        case component::x:
            return t.gather(s, spvForward<Ts>(params)..., component::x);
        case component::y:
            return t.gather(s, spvForward<Ts>(params)..., component::y);
        case component::z:
            return t.gather(s, spvForward<Ts>(params)..., component::z);
        case component::w:
            return t.gather(s, spvForward<Ts>(params)..., component::w);
    }
}

fragment void main0(constant uint* spvSwizzleConstants [[buffer(30)]], texture1d<uint> tex1d [[texture(0)]], texture2d<uint> tex2d [[texture(1)]], texture3d<uint> tex3d [[texture(2)]], texturecube<uint> texCube [[texture(3)]], texture2d_array<uint> tex2dArray [[texture(4)]], texturecube_array<uint> texCubeArray [[texture(5)]], texture2d<uint> texBuffer [[texture(6)]], sampler tex1dSmplr [[sampler(0)]], sampler tex2dSmplr [[sampler(1)]], sampler tex3dSmplr [[sampler(2)]], sampler texCubeSmplr [[sampler(3)]], sampler tex2dArraySmplr [[sampler(4)]], sampler texCubeArraySmplr [[sampler(5)]])
{
    constant uint& tex1dSwzl = spvSwizzleConstants[0];
    constant uint& tex2dSwzl = spvSwizzleConstants[1];
    constant uint& tex3dSwzl = spvSwizzleConstants[2];
    constant uint& texCubeSwzl = spvSwizzleConstants[3];
    constant uint& tex2dArraySwzl = spvSwizzleConstants[4];
    constant uint& texCubeArraySwzl = spvSwizzleConstants[5];
    float4 c = float4(spvTextureSwizzle(tex1d.sample(tex1dSmplr, 0.0), tex1dSwzl));
    c = float4(spvTextureSwizzle(tex2d.sample(tex2dSmplr, float2(0.0)), tex2dSwzl));
    c = float4(spvTextureSwizzle(tex3d.sample(tex3dSmplr, float3(0.0)), tex3dSwzl));
    c = float4(spvTextureSwizzle(texCube.sample(texCubeSmplr, float3(0.0)), texCubeSwzl));
    c = float4(spvTextureSwizzle(tex2dArray.sample(tex2dArraySmplr, float3(0.0).xy, uint(round(float3(0.0).z))), tex2dArraySwzl));
    c = float4(spvTextureSwizzle(texCubeArray.sample(texCubeArraySmplr, float4(0.0).xyz, uint(round(float4(0.0).w))), texCubeArraySwzl));
    c = float4(spvTextureSwizzle(tex1d.sample(tex1dSmplr, float2(0.0, 1.0).x / float2(0.0, 1.0).y), tex1dSwzl));
    c = float4(spvTextureSwizzle(tex2d.sample(tex2dSmplr, float3(0.0, 0.0, 1.0).xy / float3(0.0, 0.0, 1.0).z), tex2dSwzl));
    c = float4(spvTextureSwizzle(tex3d.sample(tex3dSmplr, float4(0.0, 0.0, 0.0, 1.0).xyz / float4(0.0, 0.0, 0.0, 1.0).w), tex3dSwzl));
    c = float4(spvTextureSwizzle(tex1d.sample(tex1dSmplr, 0.0), tex1dSwzl));
    c = float4(spvTextureSwizzle(tex2d.sample(tex2dSmplr, float2(0.0), level(0.0)), tex2dSwzl));
    c = float4(spvTextureSwizzle(tex3d.sample(tex3dSmplr, float3(0.0), level(0.0)), tex3dSwzl));
    c = float4(spvTextureSwizzle(texCube.sample(texCubeSmplr, float3(0.0), level(0.0)), texCubeSwzl));
    c = float4(spvTextureSwizzle(tex2dArray.sample(tex2dArraySmplr, float3(0.0).xy, uint(round(float3(0.0).z)), level(0.0)), tex2dArraySwzl));
    c = float4(spvTextureSwizzle(texCubeArray.sample(texCubeArraySmplr, float4(0.0).xyz, uint(round(float4(0.0).w)), level(0.0)), texCubeArraySwzl));
    c = float4(spvTextureSwizzle(tex1d.sample(tex1dSmplr, float2(0.0, 1.0).x / float2(0.0, 1.0).y), tex1dSwzl));
    c = float4(spvTextureSwizzle(tex2d.sample(tex2dSmplr, float3(0.0, 0.0, 1.0).xy / float3(0.0, 0.0, 1.0).z, level(0.0)), tex2dSwzl));
    c = float4(spvTextureSwizzle(tex3d.sample(tex3dSmplr, float4(0.0, 0.0, 0.0, 1.0).xyz / float4(0.0, 0.0, 0.0, 1.0).w, level(0.0)), tex3dSwzl));
    c = float4(spvTextureSwizzle(tex1d.read(uint(0)), tex1dSwzl));
    c = float4(spvTextureSwizzle(tex2d.read(uint2(int2(0)), 0), tex2dSwzl));
    c = float4(spvTextureSwizzle(tex3d.read(uint3(int3(0)), 0), tex3dSwzl));
    c = float4(spvTextureSwizzle(tex2dArray.read(uint2(int3(0).xy), uint(int3(0).z), 0), tex2dArraySwzl));
    c = float4(texBuffer.read(spvTexelBufferCoord(0)));
    c = float4(spvGatherSwizzle(tex2d, tex2dSmplr, tex2dSwzl, component::x, float2(0.0), int2(0)));
    c = float4(spvGatherSwizzle(texCube, texCubeSmplr, texCubeSwzl, component::y, float3(0.0)));
    c = float4(spvGatherSwizzle(tex2dArray, tex2dArraySmplr, tex2dArraySwzl, component::z, float3(0.0).xy, uint(round(float3(0.0).z)), int2(0)));
    c = float4(spvGatherSwizzle(texCubeArray, texCubeArraySmplr, texCubeArraySwzl, component::w, float4(0.0).xyz, uint(round(float4(0.0).w))));
}

