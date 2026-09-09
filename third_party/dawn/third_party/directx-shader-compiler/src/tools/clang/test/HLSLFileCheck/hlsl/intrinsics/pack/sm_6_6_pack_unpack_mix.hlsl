// RUN: %dxc -T ps_6_6 -enable-16bit-types %s  | FileCheck %s

// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 218, i8 1,
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 218, i8 0,
// CHECK: call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 218, i8 1,
// CHECK: call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 218, i8 0,
// CHECK: call i32 @dx.op.pack4x8.i32(i32 219, i8 2, 
// CHECK: call i32 @dx.op.pack4x8.i32(i32 219, i8 1, 
// CHECK: call i32 @dx.op.pack4x8.i16(i32 219, i8 2, 
// CHECK: call i32 @dx.op.pack4x8.i16(i32 219, i8 1, 

  %3 = call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 218, i8 1, i32 %2)  ; Unpack4x8(unpackMode,pk)
  %4 = extractvalue %dx.types.fouri32 %3, 0
  %5 = extractvalue %dx.types.fouri32 %3, 1
  %6 = extractvalue %dx.types.fouri32 %3, 2
  %7 = extractvalue %dx.types.fouri32 %3, 3
  %8 = call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 218, i8 0, i32 %1)  ; Unpack4x8(unpackMode,pk)
  %9 = extractvalue %dx.types.fouri32 %8, 0
  %10 = extractvalue %dx.types.fouri32 %8, 1
  %11 = extractvalue %dx.types.fouri32 %8, 2
  %12 = extractvalue %dx.types.fouri32 %8, 3
  %13 = call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 218, i8 1, i32 %2)  ; Unpack4x8(unpackMode,pk)
  %14 = extractvalue %dx.types.fouri16 %13, 0
  %15 = extractvalue %dx.types.fouri16 %13, 1
  %16 = extractvalue %dx.types.fouri16 %13, 2
  %17 = extractvalue %dx.types.fouri16 %13, 3
  %18 = call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 218, i8 0, i32 %1)  ; Unpack4x8(unpackMode,pk)
  %19 = extractvalue %dx.types.fouri16 %18, 0
  %20 = extractvalue %dx.types.fouri16 %18, 1
  %21 = extractvalue %dx.types.fouri16 %18, 2
  %22 = extractvalue %dx.types.fouri16 %18, 3
  %23 = call i32 @dx.op.pack4x8.i32(i32 219, i8 2, i32 %4, i32 %5, i32 %6, i32 %7)  ; Pack4x8(packMode,x,y,z,w)
  %24 = call i32 @dx.op.pack4x8.i32(i32 219, i8 1, i32 %9, i32 %10, i32 %11, i32 %12)  ; Pack4x8(packMode,x,y,z,w)
  %25 = call i32 @dx.op.pack4x8.i16(i32 219, i8 2, i16 %14, i16 %15, i16 %16, i16 %17)  ; Pack4x8(packMode,x,y,z,w)
  %26 = call i32 @dx.op.pack4x8.i16(i32 219, i8 1, i16 %19, i16 %20, i16 %21, i16 %22)  ; Pack4x8(packMode,x,y,z,w)

int4 main(int8_t4_packed input1 : Inputs1, uint8_t4_packed input2 : Inputs2) : SV_Target {
  int4 i41 = unpack_s8s32(input1);
  int4 i42 = unpack_u8u32(input2);
  int16_t4 i43 = unpack_s8s16(input1);
  uint16_t4 i44 = unpack_u8u16(input2);

  int8_t4_packed p1 = pack_clamp_s8(i41);
  uint8_t4_packed p2 = pack_clamp_u8(i42);
  int8_t4_packed p3 = pack_clamp_s8(i43);
  uint8_t4_packed p4 = pack_clamp_u8(i44);

  return p1 & p2 & p3 & p4;
}
