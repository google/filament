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

template<typename T, uint N>
inline void spvArrayCopyFromConstantToStack(thread T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToThreadGroup(threadgroup T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToStack(thread T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToThreadGroup(threadgroup T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToStack(thread T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToThreadGroup(threadgroup T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToDevice(device T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromConstantToDevice(device T (&dst)[N], constant T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromStackToDevice(device T (&dst)[N], thread const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromThreadGroupToDevice(device T (&dst)[N], threadgroup const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToStack(thread T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

template<typename T, uint N>
inline void spvArrayCopyFromDeviceToThreadGroup(threadgroup T (&dst)[N], device const T (&src)[N])
{
    for (uint i = 0; i < N; i++)
    {
        dst[i] = src[i];
    }
}

struct Block
{
    spvUnsafeArray<float, 4> block0;
};

constant spvUnsafeArray<float, 4> _30 = spvUnsafeArray<float, 4>({ 1.0, 2.0, -1.0, -2.0 });

struct main0_out
{
    float F_array_0 [[user(locn0)]];
    float F_array_1 [[user(locn1)]];
    float F_array_2 [[user(locn2)]];
    float F_array_3 [[user(locn3)]];
    float m_43_block0_0 [[user(locn4)]];
    float m_43_block0_1 [[user(locn5)]];
    float m_43_block0_2 [[user(locn6)]];
    float m_43_block0_3 [[user(locn7)]];
    float4 gl_Position [[position]];
    float gl_ClipDistance [[clip_distance]] [4];
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
    float gl_ClipDistance_2 [[user(clip2)]];
    float gl_ClipDistance_3 [[user(clip3)]];
};

static inline __attribute__((always_inline))
void in_func(thread float4& gl_Position, thread float (&gl_ClipDistance)[4], thread spvUnsafeArray<float, 4>& F_array, thread Block& _43)
{
    gl_Position = float4(1.0, 2.0, 3.0, 4.0);
    spvArrayCopyFromConstantToStack(gl_ClipDistance, _30.elements);
    spvUnsafeArray<float, 4> _36;
    _36[0] = gl_ClipDistance[0];
    _36[1] = gl_ClipDistance[1];
    _36[2] = gl_ClipDistance[2];
    _36[3] = gl_ClipDistance[3];
    spvUnsafeArray<float, 4> non_const_clips = _36;
    spvArrayCopyFromStackToStack(gl_ClipDistance, non_const_clips.elements);
    F_array = non_const_clips;
    _43.block0 = _30;
}

vertex main0_out main0()
{
    main0_out out = {};
    spvUnsafeArray<float, 4> F_array = {};
    Block _43 = {};
    in_func(out.gl_Position, out.gl_ClipDistance, F_array, _43);
    out.gl_ClipDistance_0 = out.gl_ClipDistance[0];
    out.gl_ClipDistance_1 = out.gl_ClipDistance[1];
    out.gl_ClipDistance_2 = out.gl_ClipDistance[2];
    out.gl_ClipDistance_3 = out.gl_ClipDistance[3];
    out.F_array_0 = F_array[0];
    out.F_array_1 = F_array[1];
    out.F_array_2 = F_array[2];
    out.F_array_3 = F_array[3];
    out.m_43_block0_0 = _43.block0[0];
    out.m_43_block0_1 = _43.block0[1];
    out.m_43_block0_2 = _43.block0[2];
    out.m_43_block0_3 = _43.block0[3];
    return out;
}

