// RUN: %dxilver 1.6 | %dxc -T ps_6_5 -enable-16bit-types  %s | FileCheck %s

// CHECK-DAG: 13:{{[0-9]+}}: error: Opcode Pack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 14:{{[0-9]+}}: error: Opcode Pack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 15:{{[0-9]+}}: error: Opcode Unpack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 16:{{[0-9]+}}: error: Opcode Unpack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 17:{{[0-9]+}}: error: Opcode Pack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 18:{{[0-9]+}}: error: Opcode Pack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 19:{{[0-9]+}}: error: Opcode Unpack4x8 not valid in shader model ps_6_5
// CHECK-DAG: 20:{{[0-9]+}}: error: Opcode Unpack4x8 not valid in shader model ps_6_5

int16_t4 main(int4 input1 : Inputs1, int16_t4 input2 : Inputs2) : SV_Target {
  int8_t4_packed ps1 = pack_s8(input1);
  int8_t4_packed ps2 = pack_clamp_s8(input1);
  int16_t4 up1_out = unpack_s8s16(ps1)
   + unpack_s8s16(ps2);
  uint8_t4_packed pu1 = pack_u8(input2);
  uint8_t4_packed pu2 = pack_clamp_u8(input2);
  uint16_t4 up2_out = unpack_u8u16(pu1)
    + unpack_u8u16(pu2);

  return up1_out + up2_out;
}
