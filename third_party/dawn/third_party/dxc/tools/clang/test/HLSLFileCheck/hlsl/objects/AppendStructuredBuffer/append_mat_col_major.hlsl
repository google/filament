// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=bool1x1 -DMAT1x1=1 %s | FileCheck %s -check-prefix=CHK_MAT1x1
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=float1x2 -DMAT1x2=1 %s | FileCheck %s -check-prefix=CHK_MAT1x2
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=int2x1 -DMAT2x1=1 %s | FileCheck %s -check-prefix=CHK_MAT2x1
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=uint2x2 -DMAT2x2=1 %s | FileCheck %s -check-prefix=CHK_MAT2x2
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=uint16_t2x3 -DMAT2x3=1 %s | FileCheck %s -check-prefix=CHK_MAT2x3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=int16_t3x2 -DMAT3x2=1 %s | FileCheck %s -check-prefix=CHK_MAT3x2
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=float16_t3x3 -DMAT3x3=1 %s | FileCheck %s -check-prefix=CHK_MAT3x3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=float3x4 -DMAT3x4=1 %s | FileCheck %s -check-prefix=CHK_MAT3x4
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=bool4x3 -DMAT4x3=1 %s | FileCheck %s -check-prefix=CHK_MAT4x3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -Zpc -DTY=float4x4 -DMAT4x4=1 %s | FileCheck %s -check-prefix=CHK_MAT4x4

AppendStructuredBuffer<TY> buf;

void main()
{

#ifdef MAT1x1
    bool1x1 m = bool1x1(1);
#endif    

#ifdef MAT1x2
    float1x2 m = float1x2(1, 2);
#endif

#ifdef MAT2x1
    int2x1 m = int2x1(1, 2);
#endif

#ifdef MAT2x2
    uint2x2 m = uint2x2(1, 2, 3, 4);
#endif

#ifdef MAT2x3
    uint16_t2x3 m = uint16_t2x3(1, 2, 3, 4, 5, 6);
#endif

#ifdef MAT3x2
    int16_t3x2 m = int16_t3x2(1, 2, 3, 4, 5, 6);
#endif

#ifdef MAT3x3
    float16_t3x3 m = float16_t3x3(1, 2, 3, 4, 5, 6, 7, 8, 9);
#endif

#ifdef MAT3x4
    float3x4 m = float3x4(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
#endif

#ifdef MAT4x3
    bool4x3 m = bool4x3(1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1);
#endif

#ifdef MAT4x4                          
    float4x4 m = float4x4(float4(1, 2, 3, 4), float4(5, 6, 7, 8), float4(9, 10, 11, 12), float4(13, 14, 15, 16));  
#endif

// CHK_MAT1x1: dx.op.bufferUpdateCounter
// CHK_MAT1x1: dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i32 1, i32 undef, i32 undef, i32 undef, i8 1, i32 4)

// CHK_MAT1x2: dx.op.bufferUpdateCounter
// CHK_MAT1x2: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, float 1.000000e+00, float 2.000000e+00, float undef, float undef, i8 3, i32 4)

// CHK_MAT2x1: dx.op.bufferUpdateCounter
// CHK_MAT2x1: dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i32 1, i32 2, i32 undef, i32 undef, i8 3, i32 4)

// CHK_MAT2x2: dx.op.bufferUpdateCounter
// CHK_MAT2x2: dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i32 1, i32 3, i32 2, i32 4, i8 15, i32 4)

// CHK_MAT2x3: dx.op.bufferUpdateCounter
// CHK_MAT2x3: dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i16 1, i16 4, i16 2, i16 5, i8 15, i32 2)
// CHK_MAT2x3: dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 8, i16 3, i16 6, i16 undef, i16 undef, i8 3, i32 2)

// CHK_MAT3x2: dx.op.bufferUpdateCounter
// CHK_MAT3x2: dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i16 1, i16 3, i16 5, i16 2, i8 15, i32 2)
// CHK_MAT3x2: dx.op.rawBufferStore.i16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 8, i16 4, i16 6, i16 undef, i16 undef, i8 3, i32 2)

// CHK_MAT3x3: dx.op.bufferUpdateCounter
// CHK_MAT3x3: dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, half 0xH3C00, half 0xH4400, half 0xH4700, half 0xH4000, i8 15, i32 2)
// CHK_MAT3x3: dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 8, half 0xH4500, half 0xH4800, half 0xH4200, half 0xH4600, i8 15, i32 2)
// CHK_MAT3x3: dx.op.rawBufferStore.f16(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, half 0xH4880, half undef, half undef, half undef, i8 1, i32 2)

// CHK_MAT3x4: dx.op.bufferUpdateCounter
// CHK_MAT3x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, float 1.000000e+00, float 5.000000e+00, float 9.000000e+00, float 2.000000e+00, i8 15, i32 4)
// CHK_MAT3x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, float 6.000000e+00, float 1.000000e+01, float 3.000000e+00, float 7.000000e+00, i8 15, i32 4)
// CHK_MAT3x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, float 1.100000e+01, float 4.000000e+00, float 8.000000e+00, float 1.200000e+01, i8 15, i32 4)

// CHK_MAT4x3: dx.op.bufferUpdateCounter
// CHK_MAT4x3: dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i32 1, i32 0, i32 0, i32 1, i8 15, i32 4)
// CHK_MAT4x3: dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, i32 0, i32 1, i32 1, i32 1, i8 15, i32 4)
// CHK_MAT4x3: dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, i32 0, i32 0, i32 1, i32 1, i8 15, i32 4)

// CHK_MAT4x4: dx.op.bufferUpdateCounter  
// CHK_MAT4x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, float 1.000000e+00, float 5.000000e+00, float 9.000000e+00, float 1.300000e+01, i8 15, i32 4)
// CHK_MAT4x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, float 2.000000e+00, float 6.000000e+00, float 1.000000e+01, float 1.400000e+01, i8 15, i32 4)
// CHK_MAT4x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, float 3.000000e+00, float 7.000000e+00, float 1.100000e+01, float 1.500000e+01, i8 15, i32 4) 
// CHK_MAT4x4: dx.op.rawBufferStore.f32(i32 140, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 48, float 4.000000e+00, float 8.000000e+00, float 1.200000e+01, float 1.600000e+01, i8 15, i32 4)
  
    buf.Append(m);
}