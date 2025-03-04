// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=float %s | FileCheck %s -check-prefix=CHK_SCALAR
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=float1 %s | FileCheck %s -check-prefix=CHK_VEC1
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=float1x1 %s | FileCheck %s -check-prefix=CHK_MAT1x1
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=bool1x2 %s | FileCheck %s -check-prefix=CHK_MAT1x2
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=int2x1 %s | FileCheck %s -check-prefix=CHK_MAT2x1
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=uint2x2 %s | FileCheck %s -check-prefix=CHK_MAT2x2
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=float16_t2x3 %s | FileCheck %s -check-prefix=CHK_MAT2x3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=uint16_t3x2 %s | FileCheck %s -check-prefix=CHK_MAT3x2
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=float3x3 %s | FileCheck %s -check-prefix=CHK_MAT3x3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=int3x4 %s | FileCheck %s -check-prefix=CHK_MAT3x4
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=bool4x3 %s | FileCheck %s -check-prefix=CHK_MAT4x3
// RUN: %dxc -E main -T vs_6_2 -enable-16bit-types -DTY=uint4x4 %s | FileCheck %s -check-prefix=CHK_MAT4x4


// CHK_SCALAR: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 1, i32 4)

// CHK_VEC1: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 1, i32 4)

// CHK_MAT1x1: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 1, i32 4)

// CHK_MAT1x2: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 3, i32 4)

// CHK_MAT2x1: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 3, i32 4)

// CHK_MAT2x2: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 4)

// CHK_MAT2x3: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 2)
// CHK_MAT2x3: call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 8, i8 3, i32 2)

// CHK_MAT3x2: call %dx.types.ResRet.i16 @dx.op.rawBufferLoad.i16(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 2)
// CHK_MAT3x2: call %dx.types.ResRet.i16 @dx.op.rawBufferLoad.i16(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 8, i8 3, i32 2)

// CHK_MAT3x3: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 4)
// CHK_MAT3x3: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, i8 15, i32 4)
// CHK_MAT3x3: call %dx.types.ResRet.f32 @dx.op.rawBufferLoad.f32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, i8 1, i32 4)

// CHK_MAT3x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 4)
// CHK_MAT3x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, i8 15, i32 4)
// CHK_MAT3x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, i8 15, i32 4)

// CHK_MAT4x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 4)
// CHK_MAT4x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, i8 15, i32 4)
// CHK_MAT4x3: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, i8 15, i32 4)

// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 0, i8 15, i32 4)
// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 16, i8 15, i32 4)
// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 32, i8 15, i32 4)
// CHK_MAT4x4: call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %{{.*}}, i32 %{{.*}}, i32 48, i8 15, i32 4)

StructuredBuffer<TY> stbuf;

TY main (int i : IN0) : OUT {
  return stbuf[i];
}