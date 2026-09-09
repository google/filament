// RUN: %dxc -T cs_6_2 -E main -enable-16bit-types -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

ByteAddressBuffer buf;
RWByteAddressBuffer buf2;

struct T {
  float16_t x[5];
};

struct S {
  float16_t3 a[3];
  double c;
  T t;
  double b;
  float16_t d;
};

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadId) {
  S sArr[2] = buf.Load<S[2]>(tid.x);
}

// Here is the DXIL output with the load offsets:
//
// define void @main() {
//   %buf2_UAV_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 1, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
//   %buf_texture_rawbuf = call %dx.types.Handle @dx.op.createHandle(i32 57, i8 0, i32 0, i32 0, i1 false)  ; CreateHandle(resourceClass,rangeId,index,nonUniformIndex)
//   %1 = call i32 @dx.op.threadId.i32(i32 93, i32 0)  ; ThreadId(component)
//   %RawBufferLoad46 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %1, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %2 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad46, 0
//   %3 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad46, 1
//   %4 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad46, 2
//   %5 = add i32 %1, 6
//   %RawBufferLoad45 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %5, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %6 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad45, 0
//   %7 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad45, 1
//   %8 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad45, 2
//   %9 = add i32 %1, 12
//   %RawBufferLoad44 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %9, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %10 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad44, 0
//   %11 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad44, 1
//   %12 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad44, 2
//   %13 = add i32 %1, 24
//   %14 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %13, i32 undef, i8 3, i32 8)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %15 = extractvalue %dx.types.ResRet.i32 %14, 0
//   %16 = extractvalue %dx.types.ResRet.i32 %14, 1
//   %17 = call double @dx.op.makeDouble.f64(i32 101, i32 %15, i32 %16)  ; MakeDouble(lo,hi)
//   %18 = add i32 %1, 32
//   %RawBufferLoad43 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %18, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %19 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad43, 0
//   %20 = add i32 %1, 34
//   %RawBufferLoad42 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %20, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %21 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad42, 0
//   %22 = add i32 %1, 36
//   %RawBufferLoad41 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %22, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %23 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad41, 0
//   %24 = add i32 %1, 38
//   %RawBufferLoad40 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %24, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %25 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad40, 0
//   %26 = add i32 %1, 40
//   %RawBufferLoad39 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %26, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %27 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad39, 0
//   %28 = add i32 %1, 48
//   %29 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %28, i32 undef, i8 3, i32 8)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %30 = extractvalue %dx.types.ResRet.i32 %29, 0
//   %31 = extractvalue %dx.types.ResRet.i32 %29, 1
//   %32 = call double @dx.op.makeDouble.f64(i32 101, i32 %30, i32 %31)  ; MakeDouble(lo,hi)
//   %33 = add i32 %1, 56
//   %RawBufferLoad53 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %33, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %34 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad53, 0
//   %35 = add i32 %1, 64
//   %RawBufferLoad49 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %35, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %36 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad49, 0
//   %37 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad49, 1
//   %38 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad49, 2
//   %39 = add i32 %1, 70
//   %RawBufferLoad48 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %39, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %40 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad48, 0
//   %41 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad48, 1
//   %42 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad48, 2
//   %43 = add i32 %1, 76
//   %RawBufferLoad47 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %43, i32 undef, i8 7, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %44 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad47, 0
//   %45 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad47, 1
//   %46 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad47, 2
//   %47 = add i32 %1, 88
//   %48 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %47, i32 undef, i8 3, i32 8)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %49 = extractvalue %dx.types.ResRet.i32 %48, 0
//   %50 = extractvalue %dx.types.ResRet.i32 %48, 1
//   %51 = call double @dx.op.makeDouble.f64(i32 101, i32 %49, i32 %50)  ; MakeDouble(lo,hi)
//   %52 = add i32 %1, 96
//   %RawBufferLoad38 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %52, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %53 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad38, 0
//   %54 = add i32 %1, 98
//   %RawBufferLoad37 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %54, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %55 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad37, 0
//   %56 = add i32 %1, 100
//   %RawBufferLoad36 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %56, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %57 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad36, 0
//   %58 = add i32 %1, 102
//   %RawBufferLoad35 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %58, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %59 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad35, 0
//   %60 = add i32 %1, 104
//   %RawBufferLoad = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %60, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %61 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad, 0
//   %62 = add i32 %1, 112
//   %63 = call %dx.types.ResRet.i32 @dx.op.rawBufferLoad.i32(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %62, i32 undef, i8 3, i32 8)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %64 = extractvalue %dx.types.ResRet.i32 %63, 0
//   %65 = extractvalue %dx.types.ResRet.i32 %63, 1
//   %66 = call double @dx.op.makeDouble.f64(i32 101, i32 %64, i32 %65)  ; MakeDouble(lo,hi)
//   %67 = add i32 %1, 120
//   %RawBufferLoad50 = call %dx.types.ResRet.f16 @dx.op.rawBufferLoad.f16(i32 139, %dx.types.Handle %buf_texture_rawbuf, i32 %67, i32 undef, i8 1, i32 2)  ; RawBufferLoad(srv,index,elementOffset,mask,alignment)
//   %68 = extractvalue %dx.types.ResRet.f16 %RawBufferLoad50, 0
//  ...
// }

// CHECK:    [[tidX:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %tid %int_0
// CHECK: [[basePtr:%[0-9]+]] = OpLoad %uint [[tidX]]
// CHECK:   [[index:%[0-9]+]] = OpShiftRightLogical %uint [[basePtr]] %uint_2
//
// Access to member 0 starts at offset 0.
//
// CHECK: OpAccessChain %_ptr_Uniform_uint %buf %uint_0 [[index]]
//
// Access to member 1 starts at offset 24 bytes (6 words).
//
// CHECK: OpIAdd %uint [[basePtr]] %uint_24
//
// Access to member 2 starts at offset 32 bytes (8 words).
//
// CHECK: OpIAdd %uint [[basePtr]] %uint_32
//
// Access to member 3 starts at offset 48 bytes (12 words).
//
// CHECK: OpIAdd %uint [[basePtr]] %uint_48
//
// Access to member 4 starts at offset 56 bytes (14 words).
//
// CHECK: OpIAdd %uint [[basePtr]] %uint_56
//
// The offset at the end of the last member is 58 bytes
// (the last member is fp16, so 56+2=58).
// Since we have an array of structs, the first element of the next struct
// should start at an aligned offset. Since the struct alignment is 8,
// the first member of the next struct should start at offset 64.
//
// Member 0 of the second struct starts at offset 64 bytes (16 words).
//
// CHECK: [[secondStructPtr:%[0-9]+]] = OpIAdd %uint [[basePtr]] %uint_64
//
// The rest of the members have offsets similar to the previous struct,
// But the base addres is different.
//
// CHECK: OpIAdd %uint [[secondStructPtr]] %uint_24
// CHECK: OpIAdd %uint [[secondStructPtr]] %uint_32
// CHECK: OpIAdd %uint [[secondStructPtr]] %uint_48
// CHECK: OpIAdd %uint [[secondStructPtr]] %uint_56

