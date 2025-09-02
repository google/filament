#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

template<typename Tex, typename... Tp>
using spvGatherReturn = decltype(declval<Tex>().gather(declval<sampler>(), declval<Tp>()...));

// Wrapper function that processes a device texture gather with a constant offset array.
template<typename Tex, typename Toff, typename... Tp>
inline spvGatherReturn<Tex, Tp...> spvGatherConstOffsets(const device Tex& t, sampler s, Toff coffsets, component c, Tp... params) METAL_CONST_ARG(c)
{
    spvGatherReturn<Tex, Tp...> rslts[4];
    for (uint i = 0; i < 4; i++)
    {
        switch (c)
        {
            case component::x:
                rslts[i] = t.gather(s, params..., coffsets[i], component::x);
                break;
            case component::y:
                rslts[i] = t.gather(s, params..., coffsets[i], component::y);
                break;
            case component::z:
                rslts[i] = t.gather(s, params..., coffsets[i], component::z);
                break;
            case component::w:
                rslts[i] = t.gather(s, params..., coffsets[i], component::w);
                break;
        }
    }
    return spvGatherReturn<Tex, Tp...>(rslts[0].w, rslts[1].w, rslts[2].w, rslts[3].w);
}

// Wrapper function that processes a constant texture gather with a constant offset array.
template<typename Tex, typename Toff, typename... Tp>
inline spvGatherReturn<Tex, Tp...> spvGatherConstOffsets(const constant Tex& t, sampler s, Toff coffsets, component c, Tp... params) METAL_CONST_ARG(c)
{
    spvGatherReturn<Tex, Tp...> rslts[4];
    for (uint i = 0; i < 4; i++)
    {
        switch (c)
        {
            case component::x:
                rslts[i] = t.gather(s, params..., coffsets[i], component::x);
                break;
            case component::y:
                rslts[i] = t.gather(s, params..., coffsets[i], component::y);
                break;
            case component::z:
                rslts[i] = t.gather(s, params..., coffsets[i], component::z);
                break;
            case component::w:
                rslts[i] = t.gather(s, params..., coffsets[i], component::w);
                break;
        }
    }
    return spvGatherReturn<Tex, Tp...>(rslts[0].w, rslts[1].w, rslts[2].w, rslts[3].w);
}

// Wrapper function that processes a thread texture gather with a constant offset array.
template<typename Tex, typename Toff, typename... Tp>
inline spvGatherReturn<Tex, Tp...> spvGatherConstOffsets(const thread Tex& t, sampler s, Toff coffsets, component c, Tp... params) METAL_CONST_ARG(c)
{
    spvGatherReturn<Tex, Tp...> rslts[4];
    for (uint i = 0; i < 4; i++)
    {
        switch (c)
        {
            case component::x:
                rslts[i] = t.gather(s, params..., coffsets[i], component::x);
                break;
            case component::y:
                rslts[i] = t.gather(s, params..., coffsets[i], component::y);
                break;
            case component::z:
                rslts[i] = t.gather(s, params..., coffsets[i], component::z);
                break;
            case component::w:
                rslts[i] = t.gather(s, params..., coffsets[i], component::w);
                break;
        }
    }
    return spvGatherReturn<Tex, Tp...>(rslts[0].w, rslts[1].w, rslts[2].w, rslts[3].w);
}

constant spvUnsafeArray<int2, 4> _30 = spvUnsafeArray<int2, 4>({ int2(-8), int2(-8, 7), int2(7, -8), int2(7) });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 coord [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d_array<float> tex [[texture(0)]], sampler texSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = spvGatherConstOffsets(tex, texSmplr, _30, component::y, in.coord.xy, uint(rint(in.coord.z)));
    return out;
}

