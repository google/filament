// RUN: %dxc -T ps_6_6 -enable-16bit-types  %s | FileCheck %s

//CHECK: error: no matching function for call to 'pack_clamp_s8'
//CHECK: note: candidate function not viable: no known conversion from 'vector<uint, 4>' to 'vector<int, 4>' for 1st argument
//CHECK: error: no matching function for call to 'pack_clamp_u8'
//CHECK: note: candidate function not viable: no known conversion from 'vector<uint16_t, 4>' to 'vector<int, 4>' for 1st argument

int main(uint4 input1 : Inputs1, uint16_t4 input2 : Inputs2) : SV_Target {
  int8_t4_packed ps1 = pack_s8(input1);
  int8_t4_packed ps2 = pack_clamp_s8(input1);
  uint8_t4_packed pu1 = pack_u8(input2);
  uint8_t4_packed pu2 = pack_clamp_u8(input2);

  return ps1 + ps2 + pu1 + pu2;
}
