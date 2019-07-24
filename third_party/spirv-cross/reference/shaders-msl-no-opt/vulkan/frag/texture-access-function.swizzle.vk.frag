#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 fragColor [[color(0)]];
};

// Returns 2D texture coords corresponding to 1D texel buffer coords
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
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
template<typename T, typename Tex, typename... Ts>
inline vec<T, 4> spvGatherSwizzle(sampler s, const thread Tex& t, Ts... params, component c, uint sw) METAL_CONST_ARG(c)
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

// Wrapper function that swizzles depth texture gathers.
template<typename T, typename Tex, typename... Ts>
inline vec<T, 4> spvGatherCompareSwizzle(sampler s, const thread Tex& t, Ts... params, uint sw) 
{
    if (sw)
    {
        switch (spvSwizzle(sw & 0xFF))
        {
            case spvSwizzle::none:
            case spvSwizzle::red:
                break;
            case spvSwizzle::zero:
            case spvSwizzle::green:
            case spvSwizzle::blue:
            case spvSwizzle::alpha:
                return vec<T, 4>(0, 0, 0, 0);
            case spvSwizzle::one:
                return vec<T, 4>(1, 1, 1, 1);
        }
    }
    return t.gather_compare(s, spvForward<Ts>(params)...);
}

float4 do_samples(thread const texture1d<float> t1, thread const sampler t1Smplr, constant uint& t1Swzl, thread const texture2d<float> t2, constant uint& t2Swzl, thread const texture3d<float> t3, thread const sampler t3Smplr, constant uint& t3Swzl, thread const texturecube<float> tc, constant uint& tcSwzl, thread const texture2d_array<float> t2a, thread const sampler t2aSmplr, constant uint& t2aSwzl, thread const texturecube_array<float> tca, thread const sampler tcaSmplr, constant uint& tcaSwzl, thread const texture2d<float> tb, thread const depth2d<float> d2, thread const sampler d2Smplr, constant uint& d2Swzl, thread const depthcube<float> dc, thread const sampler dcSmplr, constant uint& dcSwzl, thread const depth2d_array<float> d2a, constant uint& d2aSwzl, thread const depthcube_array<float> dca, thread const sampler dcaSmplr, constant uint& dcaSwzl, thread sampler defaultSampler, thread sampler shadowSampler)
{
    float4 c = spvTextureSwizzle(t1.sample(t1Smplr, 0.0), t1Swzl);
    c = spvTextureSwizzle(t2.sample(defaultSampler, float2(0.0)), t2Swzl);
    c = spvTextureSwizzle(t3.sample(t3Smplr, float3(0.0)), t3Swzl);
    c = spvTextureSwizzle(tc.sample(defaultSampler, float3(0.0)), tcSwzl);
    c = spvTextureSwizzle(t2a.sample(t2aSmplr, float3(0.0).xy, uint(round(float3(0.0).z))), t2aSwzl);
    c = spvTextureSwizzle(tca.sample(tcaSmplr, float4(0.0).xyz, uint(round(float4(0.0).w))), tcaSwzl);
    c.x = spvTextureSwizzle(d2.sample_compare(d2Smplr, float3(0.0, 0.0, 1.0).xy, float3(0.0, 0.0, 1.0).z), d2Swzl);
    c.x = spvTextureSwizzle(dc.sample_compare(dcSmplr, float4(0.0, 0.0, 0.0, 1.0).xyz, float4(0.0, 0.0, 0.0, 1.0).w), dcSwzl);
    c.x = spvTextureSwizzle(d2a.sample_compare(shadowSampler, float4(0.0, 0.0, 0.0, 1.0).xy, uint(round(float4(0.0, 0.0, 0.0, 1.0).z)), float4(0.0, 0.0, 0.0, 1.0).w), d2aSwzl);
    c.x = spvTextureSwizzle(dca.sample_compare(dcaSmplr, float4(0.0).xyz, uint(round(float4(0.0).w)), 1.0), dcaSwzl);
    c = spvTextureSwizzle(t1.sample(t1Smplr, float2(0.0, 1.0).x / float2(0.0, 1.0).y), t1Swzl);
    c = spvTextureSwizzle(t2.sample(defaultSampler, float3(0.0, 0.0, 1.0).xy / float3(0.0, 0.0, 1.0).z), t2Swzl);
    c = spvTextureSwizzle(t3.sample(t3Smplr, float4(0.0, 0.0, 0.0, 1.0).xyz / float4(0.0, 0.0, 0.0, 1.0).w), t3Swzl);
    float4 _119 = float4(0.0, 0.0, 1.0, 1.0);
    _119.z = float4(0.0, 0.0, 1.0, 1.0).w;
    c.x = spvTextureSwizzle(d2.sample_compare(d2Smplr, _119.xy / _119.z, float4(0.0, 0.0, 1.0, 1.0).z / _119.z), d2Swzl);
    c = spvTextureSwizzle(t1.sample(t1Smplr, 0.0), t1Swzl);
    c = spvTextureSwizzle(t2.sample(defaultSampler, float2(0.0), level(0.0)), t2Swzl);
    c = spvTextureSwizzle(t3.sample(t3Smplr, float3(0.0), level(0.0)), t3Swzl);
    c = spvTextureSwizzle(tc.sample(defaultSampler, float3(0.0), level(0.0)), tcSwzl);
    c = spvTextureSwizzle(t2a.sample(t2aSmplr, float3(0.0).xy, uint(round(float3(0.0).z)), level(0.0)), t2aSwzl);
    c = spvTextureSwizzle(tca.sample(tcaSmplr, float4(0.0).xyz, uint(round(float4(0.0).w)), level(0.0)), tcaSwzl);
    c.x = spvTextureSwizzle(d2.sample_compare(d2Smplr, float3(0.0, 0.0, 1.0).xy, float3(0.0, 0.0, 1.0).z, level(0.0)), d2Swzl);
    c = spvTextureSwizzle(t1.sample(t1Smplr, float2(0.0, 1.0).x / float2(0.0, 1.0).y), t1Swzl);
    c = spvTextureSwizzle(t2.sample(defaultSampler, float3(0.0, 0.0, 1.0).xy / float3(0.0, 0.0, 1.0).z, level(0.0)), t2Swzl);
    c = spvTextureSwizzle(t3.sample(t3Smplr, float4(0.0, 0.0, 0.0, 1.0).xyz / float4(0.0, 0.0, 0.0, 1.0).w, level(0.0)), t3Swzl);
    float4 _153 = float4(0.0, 0.0, 1.0, 1.0);
    _153.z = float4(0.0, 0.0, 1.0, 1.0).w;
    c.x = spvTextureSwizzle(d2.sample_compare(d2Smplr, _153.xy / _153.z, float4(0.0, 0.0, 1.0, 1.0).z / _153.z, level(0.0)), d2Swzl);
    c = spvTextureSwizzle(t1.read(uint(0)), t1Swzl);
    c = spvTextureSwizzle(t2.read(uint2(int2(0)), 0), t2Swzl);
    c = spvTextureSwizzle(t3.read(uint3(int3(0)), 0), t3Swzl);
    c = spvTextureSwizzle(t2a.read(uint2(int3(0).xy), uint(int3(0).z), 0), t2aSwzl);
    c = tb.read(spvTexelBufferCoord(0));
    c = spvGatherSwizzle<float, metal::texture2d<float>, float2, int2>(defaultSampler, t2, float2(0.0), int2(0), component::x, t2Swzl);
    c = spvGatherSwizzle<float, metal::texturecube<float>, float3>(defaultSampler, tc, float3(0.0), component::y, tcSwzl);
    c = spvGatherSwizzle<float, metal::texture2d_array<float>, float2, uint, int2>(t2aSmplr, t2a, float3(0.0).xy, uint(round(float3(0.0).z)), int2(0), component::z, t2aSwzl);
    c = spvGatherSwizzle<float, metal::texturecube_array<float>, float3, uint>(tcaSmplr, tca, float4(0.0).xyz, uint(round(float4(0.0).w)), component::w, tcaSwzl);
    c = spvGatherCompareSwizzle<float, metal::depth2d<float>, float2, float>(d2Smplr, d2, float2(0.0), 1.0, d2Swzl);
    c = spvGatherCompareSwizzle<float, metal::depthcube<float>, float3, float>(dcSmplr, dc, float3(0.0), 1.0, dcSwzl);
    c = spvGatherCompareSwizzle<float, metal::depth2d_array<float>, float2, uint, float>(shadowSampler, d2a, float3(0.0).xy, uint(round(float3(0.0).z)), 1.0, d2aSwzl);
    c = spvGatherCompareSwizzle<float, metal::depthcube_array<float>, float3, uint, float>(dcaSmplr, dca, float4(0.0).xyz, uint(round(float4(0.0).w)), 1.0, dcaSwzl);
    return c;
}

fragment main0_out main0(constant uint* spvSwizzleConstants [[buffer(30)]], texture1d<float> tex1d [[texture(0)]], texture2d<float> tex2d [[texture(1)]], texture3d<float> tex3d [[texture(2)]], texturecube<float> texCube [[texture(3)]], texture2d_array<float> tex2dArray [[texture(4)]], texturecube_array<float> texCubeArray [[texture(5)]], texture2d<float> texBuffer [[texture(6)]], depth2d<float> depth2d [[texture(7)]], depthcube<float> depthCube [[texture(8)]], depth2d_array<float> depth2dArray [[texture(9)]], depthcube_array<float> depthCubeArray [[texture(10)]], sampler defaultSampler [[sampler(0)]], sampler shadowSampler [[sampler(1)]], sampler tex1dSmplr [[sampler(2)]], sampler tex3dSmplr [[sampler(3)]], sampler tex2dArraySmplr [[sampler(4)]], sampler texCubeArraySmplr [[sampler(5)]], sampler depth2dSmplr [[sampler(6)]], sampler depthCubeSmplr [[sampler(7)]], sampler depthCubeArraySmplr [[sampler(8)]])
{
    main0_out out = {};
    constant uint& tex1dSwzl = spvSwizzleConstants[0];
    constant uint& tex2dSwzl = spvSwizzleConstants[1];
    constant uint& tex3dSwzl = spvSwizzleConstants[2];
    constant uint& texCubeSwzl = spvSwizzleConstants[3];
    constant uint& tex2dArraySwzl = spvSwizzleConstants[4];
    constant uint& texCubeArraySwzl = spvSwizzleConstants[5];
    constant uint& depth2dSwzl = spvSwizzleConstants[7];
    constant uint& depthCubeSwzl = spvSwizzleConstants[8];
    constant uint& depth2dArraySwzl = spvSwizzleConstants[9];
    constant uint& depthCubeArraySwzl = spvSwizzleConstants[10];
    out.fragColor = do_samples(tex1d, tex1dSmplr, tex1dSwzl, tex2d, tex2dSwzl, tex3d, tex3dSmplr, tex3dSwzl, texCube, texCubeSwzl, tex2dArray, tex2dArraySmplr, tex2dArraySwzl, texCubeArray, texCubeArraySmplr, texCubeArraySwzl, texBuffer, depth2d, depth2dSmplr, depth2dSwzl, depthCube, depthCubeSmplr, depthCubeSwzl, depth2dArray, depth2dArraySwzl, depthCubeArray, depthCubeArraySmplr, depthCubeArraySwzl, defaultSampler, shadowSampler);
    return out;
}

