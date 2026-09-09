// RUN: %dxc -T ps_6_6 %s | FileCheck %s

// CHECK: call i32 @dx.op.pack4x8.i32(i32 220, i8 0, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK: call i32 @dx.op.pack4x8.i32(i32 220, i8 2, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK-NOT: trunc
// CHECK-NOT: sext
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 219, i8 1, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 219, i8 1, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)
// CHECK-NOT: trunc
// CHECK-NOT: sext
// CHECK: call i32 @dx.op.pack4x8.i32(i32 220, i8 0, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK: call i32 @dx.op.pack4x8.i32(i32 220, i8 1, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}}, i32 %{{[0-9]+}})  ; Pack4x8(packMode,x,y,z,w)
// CHECK-NOT: trunc
// CHECK-NOT: sext
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 219, i8 0, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)
// CHECK: call %dx.types.fouri32 @dx.op.unpack4x8.i32(i32 219, i8 0, i32 %{{[0-9]+}})  ; Unpack4x8(unpackMode,pk)

int4 main(int4 input1 : Inputs1, int4 input2 : Inputs2) : SV_Target {
  int8_t4_packed ps1 = pack_s8(input1);
  int8_t4_packed ps2 = pack_clamp_s8(input1);
  int4 up1_out = unpack_s8s32(ps1) + unpack_s8s32(ps2);

  uint8_t4_packed pu1 = pack_u8(input2);
  uint8_t4_packed pu2 = pack_clamp_u8(input2);
  uint4 up2_out = unpack_u8u32(pu1) + unpack_u8u32(pu2);

  return up1_out + up2_out;
}
