#version 430

#extension GL_NV_shader_atomic_fp16_vector : enable
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : enable

layout(binding = 0) buffer Buffer
{
    f16vec2 dataf16v2;
    f16vec4 dataf16v4;

    f16vec2 resf16v2;
    f16vec4 resf16v4;

}buf;

layout(binding = 0, rg16f) volatile coherent uniform image1D        fimage1D;
layout(binding = 1, rg16f) volatile coherent uniform image1DArray   fimage1DArray;
layout(binding = 2, rg16f) volatile coherent uniform image2D        fimage2D;
layout(binding = 3, rg16f) volatile coherent uniform image2DArray   fimage2DArray;
layout(binding = 5, rg16f) volatile coherent uniform imageCube      fimageCube;
layout(binding = 6, rg16f) volatile coherent uniform imageCubeArray fimageCubeArray;
layout(binding = 9, rg16f) volatile coherent uniform image3D        fimage3D;

layout(binding = 10, rgba16f) volatile coherent uniform image1D       fimage1Dv4;
layout(binding = 11, rgba16f) volatile coherent uniform image1DArray   fimage1DArrayv4;
layout(binding = 12, rgba16f) volatile coherent uniform image2D        fimage2Dv4;
layout(binding = 13, rgba16f) volatile coherent uniform image2DArray   fimage2DArrayv4;
layout(binding = 15, rgba16f) volatile coherent uniform imageCube      fimageCubev4;
layout(binding = 16, rgba16f) volatile coherent uniform imageCubeArray fimageCubeArrayv4;
layout(binding = 19, rgba16f) volatile coherent uniform image3D        fimage3Dv4;

void main()
{
    // atomic* functions supported with f16vec2
    buf.resf16v2 = atomicAdd(buf.dataf16v2, f16vec2(3));
    buf.resf16v2 += atomicMin(buf.dataf16v2, f16vec2(3));
    buf.resf16v2 += atomicMax(buf.dataf16v2, f16vec2(3));
    buf.resf16v2 += atomicExchange(buf.dataf16v2, f16vec2(3));

    // atomic* functions supported with f16vec4
    buf.resf16v4 = atomicAdd(buf.dataf16v4, f16vec4(3));
    buf.resf16v4 += atomicMin(buf.dataf16v4, f16vec4(3));
    buf.resf16v4 += atomicMax(buf.dataf16v4, f16vec4(3));
    buf.resf16v4 += atomicExchange(buf.dataf16v4, f16vec4(3));

    // imageAtomic* functions supported with f16vec2 and only format supported is rg16f
    f16vec2 constVec2 = f16vec2(2.0);
    buf.resf16v2 += imageAtomicAdd(fimage1D, int(0), constVec2);
    buf.resf16v2 += imageAtomicAdd(fimage1DArray, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicAdd(fimage2D, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicAdd(fimage2DArray, ivec3(0,0, 0), constVec2);
    buf.resf16v2 += imageAtomicAdd(fimageCube, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicAdd(fimageCubeArray, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicAdd(fimage3D, ivec3(0,0,0), constVec2);

    buf.resf16v2 += imageAtomicMin(fimage1D, int(0), constVec2);
    buf.resf16v2 += imageAtomicMin(fimage1DArray, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicMin(fimage2D, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicMin(fimage2DArray, ivec3(0,0, 0), constVec2);
    buf.resf16v2 += imageAtomicMin(fimageCube, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicMin(fimageCubeArray, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicMin(fimage3D, ivec3(0,0,0), constVec2);

    buf.resf16v2 += imageAtomicMax(fimage1D, int(0), constVec2);
    buf.resf16v2 += imageAtomicMax(fimage1DArray, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicMax(fimage2D, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicMax(fimage2DArray, ivec3(0,0, 0), constVec2);
    buf.resf16v2 += imageAtomicMax(fimageCube, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicMax(fimageCubeArray, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicMax(fimage3D, ivec3(0,0,0), constVec2);

    buf.resf16v2 += imageAtomicExchange(fimage1D, int(0), constVec2);
    buf.resf16v2 += imageAtomicExchange(fimage1DArray, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicExchange(fimage2D, ivec2(0,0), constVec2);
    buf.resf16v2 += imageAtomicExchange(fimage2DArray, ivec3(0,0, 0), constVec2);
    buf.resf16v2 += imageAtomicExchange(fimageCube, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicExchange(fimageCubeArray, ivec3(0,0,0), constVec2);
    buf.resf16v2 += imageAtomicExchange(fimage3D, ivec3(0,0,0), constVec2);

    // imageAtomic* functions supported with f16vec4 and only format supported is rgba16f
    f16vec4 constVec4 = f16vec4(2.0);
    buf.resf16v4 += imageAtomicAdd(fimage1Dv4, int(0), constVec4);
    buf.resf16v4 += imageAtomicAdd(fimage1DArrayv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicAdd(fimage2Dv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicAdd(fimage2DArrayv4, ivec3(0,0, 0), constVec4);
    buf.resf16v4 += imageAtomicAdd(fimageCubev4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicAdd(fimageCubeArrayv4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicAdd(fimage3Dv4, ivec3(0,0,0), constVec4);

    buf.resf16v4 += imageAtomicMin(fimage1Dv4, int(0), constVec4);
    buf.resf16v4 += imageAtomicMin(fimage1DArrayv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicMin(fimage2Dv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicMin(fimage2DArrayv4, ivec3(0,0, 0), constVec4);
    buf.resf16v4 += imageAtomicMin(fimageCubev4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicMin(fimageCubeArrayv4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicMin(fimage3Dv4, ivec3(0,0,0), constVec4);

    buf.resf16v4 += imageAtomicMax(fimage1Dv4, int(0), constVec4);
    buf.resf16v4 += imageAtomicMax(fimage1DArrayv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicMax(fimage2Dv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicMax(fimage2DArrayv4, ivec3(0,0, 0), constVec4);
    buf.resf16v4 += imageAtomicMax(fimageCubev4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicMax(fimageCubeArrayv4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicMax(fimage3Dv4, ivec3(0,0,0), constVec4);

    buf.resf16v4 += imageAtomicExchange(fimage1Dv4, int(0), constVec4);
    buf.resf16v4 += imageAtomicExchange(fimage1DArrayv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicExchange(fimage2Dv4, ivec2(0,0), constVec4);
    buf.resf16v4 += imageAtomicExchange(fimage2DArrayv4, ivec3(0,0, 0), constVec4);
    buf.resf16v4 += imageAtomicExchange(fimageCubev4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicExchange(fimageCubeArrayv4, ivec3(0,0,0), constVec4);
    buf.resf16v4 += imageAtomicExchange(fimage3Dv4, ivec3(0,0,0), constVec4);
}
