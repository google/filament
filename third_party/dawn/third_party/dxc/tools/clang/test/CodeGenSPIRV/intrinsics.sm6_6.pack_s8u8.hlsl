// RUN: %dxc -E main -T ps_6_6 -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK:    %short = OpTypeInt 16 1
// CHECK:  %v4short = OpTypeVector %short 4
// CHECK:   %ushort = OpTypeInt 16 0
// CHECK: %v4ushort = OpTypeVector %ushort 4
// CHECK:     %char = OpTypeInt 8 1
// CHECK:   %v4char = OpTypeVector %char 4
// CHECK:    %uchar = OpTypeInt 8 0
// CHECK:  %v4uchar = OpTypeVector %uchar 4

float4 main(int16_t4 input1 : Inputs1, int16_t4 input2 : Inputs2) : SV_Target {
  int16_t4 v4int16_var;
  uint16_t4 v4uint16_var;

  int32_t4 v4int32_var;
  uint32_t4 v4uint32_var;

  //////////////////////
  // pack_s8 variants //
  //////////////////////

// CHECK: [[v4int16_var:%[0-9]+]] = OpLoad %v4short %v4int16_var
// CHECK:   [[bytes_vec:%[0-9]+]] = OpSConvert %v4char [[v4int16_var]]
// CHECK:      [[packed:%[0-9]+]] = OpBitcast %uint [[bytes_vec]]
// CHECK:                        OpStore %ps1 [[packed]]
  int8_t4_packed ps1 = pack_s8(v4int16_var);

// CHECK: [[v4uint16_var:%[0-9]+]] = OpLoad %v4ushort %v4uint16_var
// CHECK:    [[bytes_vec_0:%[0-9]+]] = OpUConvert %v4uchar [[v4uint16_var]]
// CHECK:       [[packed_0:%[0-9]+]] = OpBitcast %uint [[bytes_vec_0]]
// CHECK:                         OpStore %ps2 [[packed_0]]
  int8_t4_packed ps2 = pack_s8(v4uint16_var);

// CHECK: [[v4int32_var:%[0-9]+]] = OpLoad %v4int %v4int32_var
// CHECK:   [[bytes_vec_1:%[0-9]+]] = OpSConvert %v4char [[v4int32_var]]
// CHECK:      [[packed_1:%[0-9]+]] = OpBitcast %uint [[bytes_vec_1]]
// CHECK:                        OpStore %ps3 [[packed_1]]
  int8_t4_packed ps3 = pack_s8(v4int32_var);
// CHECK: [[v4uint32_var:%[0-9]+]] = OpLoad %v4uint %v4uint32_var
// CHECK:    [[bytes_vec_2:%[0-9]+]] = OpUConvert %v4uchar [[v4uint32_var]]
// CHECK:       [[packed_2:%[0-9]+]] = OpBitcast %uint [[bytes_vec_2]]
// CHECK:                         OpStore %ps4 [[packed_2]]
  int8_t4_packed ps4 = pack_s8(v4uint32_var);

  //////////////////////
  // pack_u8 variants //
  //////////////////////

// CHECK: [[v4int16_var_0:%[0-9]+]] = OpLoad %v4short %v4int16_var
// CHECK:   [[bytes_vec_3:%[0-9]+]] = OpSConvert %v4char [[v4int16_var_0]]
// CHECK:      [[packed_3:%[0-9]+]] = OpBitcast %uint [[bytes_vec_3]]
// CHECK:                        OpStore %pu1 [[packed_3]]
  uint8_t4_packed pu1 = pack_u8(v4int16_var);

// CHECK: [[v4uint16_var_0:%[0-9]+]] = OpLoad %v4ushort %v4uint16_var
// CHECK:    [[bytes_vec_4:%[0-9]+]] = OpUConvert %v4uchar [[v4uint16_var_0]]
// CHECK:       [[packed_4:%[0-9]+]] = OpBitcast %uint [[bytes_vec_4]]
// CHECK:                         OpStore %pu2 [[packed_4]]
  uint8_t4_packed pu2 = pack_u8(v4uint16_var);

// CHECK: [[v4int32_var_0:%[0-9]+]] = OpLoad %v4int %v4int32_var
// CHECK:   [[bytes_vec_5:%[0-9]+]] = OpSConvert %v4char [[v4int32_var_0]]
// CHECK:      [[packed_5:%[0-9]+]] = OpBitcast %uint [[bytes_vec_5]]
// CHECK:                        OpStore %pu3 [[packed_5]]
  uint8_t4_packed pu3 = pack_u8(v4int32_var);

// CHECK: [[v4uint32_var_0:%[0-9]+]] = OpLoad %v4uint %v4uint32_var
// CHECK:    [[bytes_vec_6:%[0-9]+]] = OpUConvert %v4uchar [[v4uint32_var_0]]
// CHECK:       [[packed_6:%[0-9]+]] = OpBitcast %uint [[bytes_vec_6]]
// CHECK:                         OpStore %pu4 [[packed_6]]
  uint8_t4_packed pu4 = pack_u8(v4uint32_var);

  return 0.xxxx;
}
