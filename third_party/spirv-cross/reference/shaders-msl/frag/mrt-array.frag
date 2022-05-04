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

struct main0_out
{
    float4 FragColor_0 [[color(0)]];
    float4 FragColor_1 [[color(1)]];
    float4 FragColor_2 [[color(2)]];
    float4 FragColor_3 [[color(3)]];
};

struct main0_in
{
    float4 vA [[user(locn0)]];
    float4 vB [[user(locn1)]];
};

static inline __attribute__((always_inline))
void write_deeper_in_function(thread spvUnsafeArray<float4, 4>& FragColor, thread float4& vA, thread float4& vB)
{
    FragColor[3] = vA * vB;
}

static inline __attribute__((always_inline))
void write_in_function(thread spvUnsafeArray<float4, 4>& FragColor, thread float4& vA, thread float4& vB)
{
    FragColor[2] = vA - vB;
    write_deeper_in_function(FragColor, vA, vB);
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 4> FragColor = {};
    FragColor[0] = mod(in.vA, in.vB);
    FragColor[1] = in.vA + in.vB;
    write_in_function(FragColor, in.vA, in.vB);
    out.FragColor_0 = FragColor[0];
    out.FragColor_1 = FragColor[1];
    out.FragColor_2 = FragColor[2];
    out.FragColor_3 = FragColor[3];
    return out;
}

