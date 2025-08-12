// RUN: %dxc -T cs_6_9 %s -enable-16bit-types -DITY=float16_t -DMI=F16 -DML=OuterProductOptimal | FileCheck %s --check-prefixes COMMON,DXIL-0
// RUN: %dxc -T cs_6_9 %s -enable-16bit-types -DITY=float16_t -DMI=F8_E4M3 -DML=OuterProductOptimal | FileCheck %s --check-prefixes COMMON,DXIL-1
// RUN: %dxc -T cs_6_9 %s -enable-16bit-types -DITY=uint -DMI=U8 -DML=OuterProductOptimal | FileCheck %s --check-prefixes COMMON,DXIL-2
// RUN: %dxc -T cs_6_9 %s -enable-16bit-types -DITY=float16_t -DMI=F16 -DML=OuterProductOptimal -fcgl | FileCheck %s --check-prefixes COMMON,HLOP-0
// RUN: %dxc -T cs_6_9 %s -enable-16bit-types -DITY=float16_t -DMI=F8_E4M3 -DML=OuterProductOptimal -fcgl | FileCheck %s --check-prefixes COMMON,HLOP-1
// RUN: %dxc -T cs_6_9 %s -enable-16bit-types -DITY=uint -DMI=U8 -DML=OuterProductOptimal -fcgl | FileCheck %s --check-prefixes COMMON,HLOP-2

ByteAddressBuffer input_vector_buffer;
ByteAddressBuffer input_vector_buffer2;
RWByteAddressBuffer matrix_buffer;

// COMMON: define void @main()

// DXIL-0: call void @dx.op.outerProductAccumulate.v8f16.v8f16(i32 307, <8 x half> %{{[^ ]+}}, <8 x half> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 8, i32 3, i32 0)  ; OuterProductAccumulate(inputVector1,inputVector2,matrixBuffer,matrixOffset,matrixIntepretation,matrixLayout,matrixStride)

// HLOP-0: call void @"dx.hl.op..void (i32, <8 x half>, <8 x half>, %dx.types.Handle, i32, i32, i32, i32)"(i32 392, <8 x half> %{{[^ ]+}}, <8 x half> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 8, i32 3, i32 0)

// DXIL-1: call void @dx.op.outerProductAccumulate.v8f16.v8f16(i32 307, <8 x half> %{{[^ ]+}}, <8 x half> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 21, i32 3, i32 0)  ; OuterProductAccumulate(inputVector1,inputVector2,matrixBuffer,matrixOffset,matrixIntepretation,matrixLayout,matrixStride)

// HLOP-1: call void @"dx.hl.op..void (i32, <8 x half>, <8 x half>, %dx.types.Handle, i32, i32, i32, i32)"(i32 392, <8 x half> %{{[^ ]+}}, <8 x half> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 21, i32 3, i32 0)

// DXIL-2: call void @dx.op.outerProductAccumulate.v8i32.v8i32(i32 307, <8 x i32> %{{[^ ]+}}, <8 x i32> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 19, i32 3, i32 0)  ; OuterProductAccumulate(inputVector1,inputVector2,matrixBuffer,matrixOffset,matrixIntepretation,matrixLayout,matrixStride)

// HLOP-2: call void @"dx.hl.op..void (i32, <8 x i32>, <8 x i32>, %dx.types.Handle, i32, i32, i32, i32)"(i32 392, <8 x i32> %{{[^ ]+}}, <8 x i32> %{{[^ ]+}}, %dx.types.Handle %{{[^ ]+}}, i32 0, i32 19, i32 3, i32 0)

enum CompType {
  Invalid = 0,
  I1 = 1,
  I16 = 2,
  U16 = 3,
  I32 = 4,
  U32 = 5,
  I64 = 6,
  U64 = 7,
  F16 = 8,
  F32 = 9,
  F64 = 10,
  SNormF16 = 11,
  UNormF16 = 12,
  SNormF32 = 13,
  UNormF32 = 14,
  SNormF64 = 15,
  UNormF64 = 16,
  PackedS8x32 = 17,
  PackedU8x32 = 18,

  // BEGIN NEW FOR SM 6.9
  U8 = 19,
  I8 = 20,
  F8_E4M3 = 21,
  F8_E5M2 = 22,
};

enum MatLayout {
  RowMajor = 0,
  ColumnMajor = 1,
  MulOptimal = 2,
  OuterProductOptimal = 3,
};


[Numthreads(1,1,1)]
void main()
{
    vector<ITY, 8> input_vector1 = input_vector_buffer.Load<vector<ITY, 8> >(0);
    vector<ITY, 8> input_vector2 = input_vector_buffer2.Load<vector<ITY, 8> >(0);

    const uint matrix_interpretation = MI;
    const uint matrix_layout = ML;
    const uint matrix_offset = 0;
    const uint matrix_stride = 0;

    __builtin_OuterProductAccumulate(input_vector1, input_vector2, matrix_buffer, matrix_offset, matrix_interpretation, matrix_layout, matrix_stride);

}
