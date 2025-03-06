// RUN: %dxc -T ps_6_6 -enable-16bit-types  %s | FileCheck %s

// CHECK: call i32 @dx.op.pack4x8.i16(i32 220, i8 0, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK: call i32 @dx.op.pack4x8.i16(i32 220, i8 2, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK-NOT: trunc
// CHECK-NOT: sext
// CHECK: call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 219, i8 1, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)
// CHECK: call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 219, i8 1, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)
// CHECK-NOT: trunc
// CHECK-NOT: sext
// CHECK: call i32 @dx.op.pack4x8.i16(i32 220, i8 0, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK: call i32 @dx.op.pack4x8.i16(i32 220, i8 1, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}}, i16 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK-NOT: trunc
// CHECK-NOT: sext
// CHECK: call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 219, i8 0, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)
// CHECK: call %dx.types.fouri16 @dx.op.unpack4x8.i16(i32 219, i8 0, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)

int16_t4 main(int16_t4 input1 : Inputs1, int16_t4 input2 : Inputs2) : SV_Target {
  int8_t4_packed ps1 = pack_s8(input1);
  int8_t4_packed ps2 = pack_clamp_s8(input1);
  int16_t4 up1_out = unpack_s8s16(ps1) + unpack_s8s16(ps2);

  uint8_t4_packed pu1 = pack_u8(input2);
  uint8_t4_packed pu2 = pack_clamp_u8(input2);
  uint16_t4 up2_out = unpack_u8u16(pu1) + unpack_u8u16(pu2);

  return up1_out + up2_out;
}
