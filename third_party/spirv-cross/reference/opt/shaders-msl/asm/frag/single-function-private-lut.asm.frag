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

// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()
template<typename Tx, typename Ty>
inline Tx mod(Tx x, Ty y)
{
    return x - y * floor(x / y);
}

struct myType
{
    float data;
};

struct main0_out
{
    float4 o_color [[color(0)]];
};

fragment main0_out main0(float4 gl_FragCoord [[position]])
{
    spvUnsafeArray<myType, 5> _21 = spvUnsafeArray<myType, 5>({ myType{ 0.0 }, myType{ 1.0 }, myType{ 0.0 }, myType{ 1.0 }, myType{ 0.0 } });
    
    main0_out out = {};
    if (_21[int(mod(gl_FragCoord.x, 4.0))].data > 0.0)
    {
        out.o_color = float4(0.0, 1.0, 0.0, 1.0);
    }
    else
    {
        out.o_color = float4(1.0, 0.0, 0.0, 1.0);
    }
    return out;
}

