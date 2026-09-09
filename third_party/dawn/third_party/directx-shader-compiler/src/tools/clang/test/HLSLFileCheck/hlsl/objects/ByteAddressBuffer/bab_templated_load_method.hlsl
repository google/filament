// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=float -DRET_TY=float %s | FileCheck %s -check-prefix=CHK_SCALAR
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=float1 -DRET_TY=float1 %s | FileCheck %s -check-prefix=CHK_VEC1
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=float1x1 -DRET_TY=float1x1 %s | FileCheck %s -check-prefix=CHK_MAT1x1
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=bool1x2 -DRET_TY=bool1x2 %s | FileCheck %s -check-prefix=CHK_MAT1x2
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=int2x1 -DRET_TY=int2x1 %s | FileCheck %s -check-prefix=CHK_MAT2x1
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=uint2x2 -DRET_TY=uint2x2 %s | FileCheck %s -check-prefix=CHK_MAT2x2
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=float16_t2x3 -DRET_TY=float16_t2x3 %s | FileCheck %s -check-prefix=CHK_MAT2x3
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=uint16_t3x2 -DRET_TY=uint16_t3x2 %s | FileCheck %s -check-prefix=CHK_MAT3x2
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=float3x3 -DRET_TY=float3x3 %s | FileCheck %s -check-prefix=CHK_MAT3x3
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=int3x4 -DRET_TY=int3x4 %s | FileCheck %s -check-prefix=CHK_MAT3x4
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=bool4x3 -DRET_TY=bool4x3 %s | FileCheck %s -check-prefix=CHK_MAT4x3
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=uint4x4 -DRET_TY=uint4x4 %s | FileCheck %s -check-prefix=CHK_MAT4x4

// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=double -DRET_TY=float %s | FileCheck %s -check-prefix=CHK_DBL
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=double4 -DRET_TY=float4 %s | FileCheck %s -check-prefix=CHK_DBL4
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=double3x3 -DRET_TY=float3x3 %s | FileCheck %s -check-prefix=CHK_DBL3x3
// RUN: %dxc -E main -T vs_6_5 -enable-16bit-types -DTY=double4x4 -DRET_TY=float4x4 %s | FileCheck %s -check-prefix=CHK_DBL4x4

// RUN: %dxc -E main -T vs_6_5 -DTY=min16float -DRET_TY=min16float %s | FileCheck %s -check-prefix=CHK_MINFLT
// RUN: %dxc -E main -T vs_6_5 -DTY=min16uint3x3 -DRET_TY=min16uint3x3 %s | FileCheck %s -check-prefix=CHK_MINUINT3x3
// RUN: %dxc -E main -T vs_6_5 -DTY=min16float4x4 -DRET_TY=min16float4x4 %s | FileCheck %s -check-prefix=CHK_MINFLT4x4


// CHK_SCALAR: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_VEC1: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_MAT1x1: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_MAT1x2: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 3, i32 4)

// CHK_MAT2x1: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 3, i32 4)

// CHK_MAT2x2: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)

// CHK_MAT2x3: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 2)
// CHK_MAT2x3: add i32 %{{.*}}, 8
// CHK_MAT2x3: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 3, i32 2)

// CHK_MAT3x2: call %dx.types.ResRet.i16 @dx.op.rawBufferLoad.i16(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 2)
// CHK_MAT3x2: add i32 %{{.*}}, 8
// CHK_MAT3x2: call %dx.types.ResRet.i16 @dx.op.rawBufferLoad.i16(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 3, i32 2)

// CHK_MAT3x3: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT3x3: add i32 %{{.*}}, 16
// CHK_MAT3x3: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT3x3: add i32 %{{.*}}, 32
// CHK_MAT3x3: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_MAT3x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT3x4: add i32 %{{.*}}, 16
// CHK_MAT3x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT3x4: add i32 %{{.*}}, 32
// CHK_MAT3x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)

// CHK_MAT4x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT4x3: add i32 %{{.*}}, 16
// CHK_MAT4x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT4x3: add i32 %{{.*}}, 32
// CHK_MAT4x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)

// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT4x4: add i32 %{{.*}}, 16
// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT4x4: add i32 %{{.*}}, 32
// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MAT4x4: add i32 %{{.*}}, 48
// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)

// CHK_DBL: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_DBL4: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)

// CHK_DBL3x3: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_DBL3x3: add i32 %{{.*}}, 32
// CHK_DBL3x3: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_DBL3x3: add i32 %{{.*}}, 64
// CHK_DBL3x3: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_DBL4x4: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_DBL4x4: add i32 %{{.*}}, 32
// CHK_DBL4x4: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_DBL4x4: add i32 %{{.*}}, 64
// CHK_DBL4x4: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_DBL4x4: add i32 %{{.*}}, 96
// CHK_DBL4x4: call %dx.types.ResRet.f64 @dx.op.rawBufferLoad.f64(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)

// CHK_MINFLT: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_MINUINT3x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MINUINT3x3: add i32 %{{.*}}, 16
// CHK_MINUINT3x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MINUINT3x3: add i32 %{{.*}}, 32
// CHK_MINUINT3x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 undef, i8 1, i32 4)

// CHK_MINFLT4x4: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MINFLT4x4: add i32 %{{.*}}, 16
// CHK_MINFLT4x4: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MINFLT4x4: add i32 %{{.*}}, 32
// CHK_MINFLT4x4: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)
// CHK_MINFLT4x4: add i32 %{{.*}}, 48
// CHK_MINFLT4x4: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %bab_texture_rawbuf, i32 %{{.*}}, i32 undef, i8 15, i32 4)

ByteAddressBuffer bab;

RET_TY main (int i : IN0) : OUT {
  return bab.Load<TY>(i);
}