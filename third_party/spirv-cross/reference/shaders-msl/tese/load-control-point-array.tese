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

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vInputs [[attribute(0)]];
};

struct main0_patchIn
{
    float4 vBoo_0 [[attribute(1)]];
    float4 vBoo_1 [[attribute(2)]];
    float4 vBoo_2 [[attribute(3)]];
    float4 vBoo_3 [[attribute(4)]];
    int vIndex [[attribute(5)]];
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 4> vBoo = {};
    vBoo[0] = patchIn.vBoo_0;
    vBoo[1] = patchIn.vBoo_1;
    vBoo[2] = patchIn.vBoo_2;
    vBoo[3] = patchIn.vBoo_3;
    spvUnsafeArray<float4, 32> _15 = spvUnsafeArray<float4, 32>({ patchIn.gl_in[0].vInputs, patchIn.gl_in[1].vInputs, patchIn.gl_in[2].vInputs, patchIn.gl_in[3].vInputs, patchIn.gl_in[4].vInputs, patchIn.gl_in[5].vInputs, patchIn.gl_in[6].vInputs, patchIn.gl_in[7].vInputs, patchIn.gl_in[8].vInputs, patchIn.gl_in[9].vInputs, patchIn.gl_in[10].vInputs, patchIn.gl_in[11].vInputs, patchIn.gl_in[12].vInputs, patchIn.gl_in[13].vInputs, patchIn.gl_in[14].vInputs, patchIn.gl_in[15].vInputs, patchIn.gl_in[16].vInputs, patchIn.gl_in[17].vInputs, patchIn.gl_in[18].vInputs, patchIn.gl_in[19].vInputs, patchIn.gl_in[20].vInputs, patchIn.gl_in[21].vInputs, patchIn.gl_in[22].vInputs, patchIn.gl_in[23].vInputs, patchIn.gl_in[24].vInputs, patchIn.gl_in[25].vInputs, patchIn.gl_in[26].vInputs, patchIn.gl_in[27].vInputs, patchIn.gl_in[28].vInputs, patchIn.gl_in[29].vInputs, patchIn.gl_in[30].vInputs, patchIn.gl_in[31].vInputs });
    spvUnsafeArray<float4, 32> tmp;
    tmp = _15;
    out.gl_Position = (tmp[0] + tmp[1]) + vBoo[patchIn.vIndex];
    return out;
}

