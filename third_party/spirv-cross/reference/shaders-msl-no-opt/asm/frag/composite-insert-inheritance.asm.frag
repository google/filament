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

constant float4 _33 = {};

constant spvUnsafeArray<float4, 2> _35 = spvUnsafeArray<float4, 2>({ float4(0.0), float4(0.0) });

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vInput [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float4 _37 = in.vInput;
    float4 _38 = _37;
    _38.x = 1.0;
    _38.y = 2.0;
    _38.z = 3.0;
    _38.w = 4.0;
    out.FragColor = _38;
    float4 _8 = _37;
    _8.x = 1.0;
    _8.y = 2.0;
    _8.z = 3.0;
    _8.w = 4.0;
    out.FragColor = _8;
    float4 _42 = _37;
    _42.x = 1.0;
    _42.y = 2.0;
    _42.z = 3.0;
    _42.w = 4.0;
    out.FragColor = _42;
    float4 _44 = _37;
    _44.x = 1.0;
    float4 _45 = _44;
    _45.y = 2.0;
    float4 _46 = _45;
    _46.z = 3.0;
    float4 _47 = _46;
    _47.w = 4.0;
    out.FragColor = _47 + _44;
    out.FragColor = _47 + _45;
    float4 _49;
    _49.x = 1.0;
    _49.y = 2.0;
    _49.z = 3.0;
    _49.w = 4.0;
    out.FragColor = _49;
    float4 _53 = float4(0.0);
    _53.x = 1.0;
    out.FragColor = _53;
    spvUnsafeArray<float4, 2> _54 = _35;
    _54[1].z = 1.0;
    _54[0].w = 2.0;
    out.FragColor = _54[0];
    out.FragColor = _54[1];
    float4x4 _58 = float4x4(float4(0.0), float4(0.0), float4(0.0), float4(0.0));
    _58[1].z = 1.0;
    _58[2].w = 2.0;
    out.FragColor = _58[0];
    out.FragColor = _58[1];
    out.FragColor = _58[2];
    out.FragColor = _58[3];
    float4 PHI;
    PHI = _46;
    float4 _65 = PHI;
    _65.w = 4.0;
    out.FragColor = _65;
    return out;
}

