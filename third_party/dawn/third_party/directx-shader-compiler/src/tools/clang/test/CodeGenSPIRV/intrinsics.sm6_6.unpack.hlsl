// RUN: %dxc -E main -T ps_6_6 -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

float4 main(int16_t4 input1 : Inputs1, int16_t4 input2 : Inputs2) : SV_Target {
  // Note: both int8_t4_packed and uint8_t4_packed are represented as
  // 32-bit unsigned integers in SPIR-V.
  int8_t4_packed signedPacked;
  uint8_t4_packed unsignedPacked;

// CHECK:    [[packed:%[0-9]+]] = OpLoad %uint %unsignedPacked
// CHECK: [[bytes_vec:%[0-9]+]] = OpBitcast %v4char [[packed]]
// CHECK:  [[unpacked:%[0-9]+]] = OpSConvert %v4short [[bytes_vec]]
// CHECK:                      OpStore %up1 [[unpacked]]
  int16_t4 up1 = unpack_s8s16(unsignedPacked);

// CHECK:    [[packed_0:%[0-9]+]] = OpLoad %uint %signedPacked
// CHECK: [[bytes_vec_0:%[0-9]+]] = OpBitcast %v4char [[packed_0]]
// CHECK:  [[unpacked_0:%[0-9]+]] = OpSConvert %v4short [[bytes_vec_0]]
// CHECK:                      OpStore %up2 [[unpacked_0]]
  int16_t4 up2 = unpack_s8s16(signedPacked);

// CHECK:    [[packed_1:%[0-9]+]] = OpLoad %uint %unsignedPacked
// CHECK: [[bytes_vec_1:%[0-9]+]] = OpBitcast %v4char [[packed_1]]
// CHECK:  [[unpacked_1:%[0-9]+]] = OpSConvert %v4int [[bytes_vec_1]]
// CHECK:                      OpStore %up3 [[unpacked_1]]
  int32_t4 up3 = unpack_s8s32(unsignedPacked);

// CHECK:    [[packed_2:%[0-9]+]] = OpLoad %uint %signedPacked
// CHECK: [[bytes_vec_2:%[0-9]+]] = OpBitcast %v4char [[packed_2]]
// CHECK:  [[unpacked_2:%[0-9]+]] = OpSConvert %v4int [[bytes_vec_2]]
// CHECK:                      OpStore %up4 [[unpacked_2]]
  int32_t4 up4 = unpack_s8s32(signedPacked);

// CHECK:    [[packed_3:%[0-9]+]] = OpLoad %uint %unsignedPacked
// CHECK: [[bytes_vec_3:%[0-9]+]] = OpBitcast %v4uchar [[packed_3]]
// CHECK:  [[unpacked_3:%[0-9]+]] = OpUConvert %v4ushort [[bytes_vec_3]]
// CHECK:                      OpStore %up5 [[unpacked_3]]
  uint16_t4 up5 = unpack_u8u16(unsignedPacked);

// CHECK:    [[packed_4:%[0-9]+]] = OpLoad %uint %signedPacked
// CHECK: [[bytes_vec_4:%[0-9]+]] = OpBitcast %v4uchar [[packed_4]]
// CHECK:  [[unpacked_4:%[0-9]+]] = OpUConvert %v4ushort [[bytes_vec_4]]
// CHECK:                      OpStore %up6 [[unpacked_4]]
  uint16_t4 up6 = unpack_u8u16(signedPacked);

// CHECK:    [[packed_5:%[0-9]+]] = OpLoad %uint %unsignedPacked
// CHECK: [[bytes_vec_5:%[0-9]+]] = OpBitcast %v4uchar [[packed_5]]
// CHECK:  [[unpacked_5:%[0-9]+]] = OpUConvert %v4uint [[bytes_vec_5]]
// CHECK:                      OpStore %up7 [[unpacked_5]]
  uint32_t4 up7 = unpack_u8u32(unsignedPacked);

// CHECK:    [[packed_6:%[0-9]+]] = OpLoad %uint %signedPacked
// CHECK: [[bytes_vec_6:%[0-9]+]] = OpBitcast %v4uchar [[packed_6]]
// CHECK:  [[unpacked_6:%[0-9]+]] = OpUConvert %v4uint [[bytes_vec_6]]
// CHECK:                      OpStore %up8 [[unpacked_6]]
  uint32_t4 up8 = unpack_u8u32(signedPacked);

  return 0.xxxx;
}
