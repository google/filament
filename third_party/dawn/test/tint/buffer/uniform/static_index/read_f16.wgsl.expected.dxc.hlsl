int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

struct Inner {
  int scalar_i32;
  float scalar_f32;
  float16_t scalar_f16;
};

cbuffer cbuffer_ub : register(b0) {
  uint4 ub[55];
};
RWByteAddressBuffer s : register(u1);

float2x2 ub_load_16(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = ub[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = ub[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

float2x3 ub_load_17(uint offset) {
  const uint scalar_offset_2 = ((offset + 0u)) / 4;
  const uint scalar_offset_3 = ((offset + 16u)) / 4;
  return float2x3(asfloat(ub[scalar_offset_2 / 4].xyz), asfloat(ub[scalar_offset_3 / 4].xyz));
}

float2x4 ub_load_18(uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 16u)) / 4;
  return float2x4(asfloat(ub[scalar_offset_4 / 4]), asfloat(ub[scalar_offset_5 / 4]));
}

float3x2 ub_load_19(uint offset) {
  const uint scalar_offset_6 = ((offset + 0u)) / 4;
  uint4 ubo_load_2 = ub[scalar_offset_6 / 4];
  const uint scalar_offset_7 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = ub[scalar_offset_7 / 4];
  const uint scalar_offset_8 = ((offset + 16u)) / 4;
  uint4 ubo_load_4 = ub[scalar_offset_8 / 4];
  return float3x2(asfloat(((scalar_offset_6 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_7 & 2) ? ubo_load_3.zw : ubo_load_3.xy)), asfloat(((scalar_offset_8 & 2) ? ubo_load_4.zw : ubo_load_4.xy)));
}

float3x3 ub_load_20(uint offset) {
  const uint scalar_offset_9 = ((offset + 0u)) / 4;
  const uint scalar_offset_10 = ((offset + 16u)) / 4;
  const uint scalar_offset_11 = ((offset + 32u)) / 4;
  return float3x3(asfloat(ub[scalar_offset_9 / 4].xyz), asfloat(ub[scalar_offset_10 / 4].xyz), asfloat(ub[scalar_offset_11 / 4].xyz));
}

float3x4 ub_load_21(uint offset) {
  const uint scalar_offset_12 = ((offset + 0u)) / 4;
  const uint scalar_offset_13 = ((offset + 16u)) / 4;
  const uint scalar_offset_14 = ((offset + 32u)) / 4;
  return float3x4(asfloat(ub[scalar_offset_12 / 4]), asfloat(ub[scalar_offset_13 / 4]), asfloat(ub[scalar_offset_14 / 4]));
}

float4x2 ub_load_22(uint offset) {
  const uint scalar_offset_15 = ((offset + 0u)) / 4;
  uint4 ubo_load_5 = ub[scalar_offset_15 / 4];
  const uint scalar_offset_16 = ((offset + 8u)) / 4;
  uint4 ubo_load_6 = ub[scalar_offset_16 / 4];
  const uint scalar_offset_17 = ((offset + 16u)) / 4;
  uint4 ubo_load_7 = ub[scalar_offset_17 / 4];
  const uint scalar_offset_18 = ((offset + 24u)) / 4;
  uint4 ubo_load_8 = ub[scalar_offset_18 / 4];
  return float4x2(asfloat(((scalar_offset_15 & 2) ? ubo_load_5.zw : ubo_load_5.xy)), asfloat(((scalar_offset_16 & 2) ? ubo_load_6.zw : ubo_load_6.xy)), asfloat(((scalar_offset_17 & 2) ? ubo_load_7.zw : ubo_load_7.xy)), asfloat(((scalar_offset_18 & 2) ? ubo_load_8.zw : ubo_load_8.xy)));
}

float4x3 ub_load_23(uint offset) {
  const uint scalar_offset_19 = ((offset + 0u)) / 4;
  const uint scalar_offset_20 = ((offset + 16u)) / 4;
  const uint scalar_offset_21 = ((offset + 32u)) / 4;
  const uint scalar_offset_22 = ((offset + 48u)) / 4;
  return float4x3(asfloat(ub[scalar_offset_19 / 4].xyz), asfloat(ub[scalar_offset_20 / 4].xyz), asfloat(ub[scalar_offset_21 / 4].xyz), asfloat(ub[scalar_offset_22 / 4].xyz));
}

float4x4 ub_load_24(uint offset) {
  const uint scalar_offset_23 = ((offset + 0u)) / 4;
  const uint scalar_offset_24 = ((offset + 16u)) / 4;
  const uint scalar_offset_25 = ((offset + 32u)) / 4;
  const uint scalar_offset_26 = ((offset + 48u)) / 4;
  return float4x4(asfloat(ub[scalar_offset_23 / 4]), asfloat(ub[scalar_offset_24 / 4]), asfloat(ub[scalar_offset_25 / 4]), asfloat(ub[scalar_offset_26 / 4]));
}

matrix<float16_t, 2, 2> ub_load_25(uint offset) {
  const uint scalar_offset_27 = ((offset + 0u)) / 4;
  uint ubo_load_9 = ub[scalar_offset_27 / 4][scalar_offset_27 % 4];
  const uint scalar_offset_28 = ((offset + 4u)) / 4;
  uint ubo_load_10 = ub[scalar_offset_28 / 4][scalar_offset_28 % 4];
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_9 & 0xFFFF)), float16_t(f16tof32(ubo_load_9 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_10 & 0xFFFF)), float16_t(f16tof32(ubo_load_10 >> 16))));
}

matrix<float16_t, 2, 3> ub_load_26(uint offset) {
  const uint scalar_offset_29 = ((offset + 0u)) / 4;
  uint4 ubo_load_12 = ub[scalar_offset_29 / 4];
  uint2 ubo_load_11 = ((scalar_offset_29 & 2) ? ubo_load_12.zw : ubo_load_12.xy);
  vector<float16_t, 2> ubo_load_11_xz = vector<float16_t, 2>(f16tof32(ubo_load_11 & 0xFFFF));
  float16_t ubo_load_11_y = f16tof32(ubo_load_11[0] >> 16);
  const uint scalar_offset_30 = ((offset + 8u)) / 4;
  uint4 ubo_load_14 = ub[scalar_offset_30 / 4];
  uint2 ubo_load_13 = ((scalar_offset_30 & 2) ? ubo_load_14.zw : ubo_load_14.xy);
  vector<float16_t, 2> ubo_load_13_xz = vector<float16_t, 2>(f16tof32(ubo_load_13 & 0xFFFF));
  float16_t ubo_load_13_y = f16tof32(ubo_load_13[0] >> 16);
  return matrix<float16_t, 2, 3>(vector<float16_t, 3>(ubo_load_11_xz[0], ubo_load_11_y, ubo_load_11_xz[1]), vector<float16_t, 3>(ubo_load_13_xz[0], ubo_load_13_y, ubo_load_13_xz[1]));
}

matrix<float16_t, 2, 4> ub_load_27(uint offset) {
  const uint scalar_offset_31 = ((offset + 0u)) / 4;
  uint4 ubo_load_16 = ub[scalar_offset_31 / 4];
  uint2 ubo_load_15 = ((scalar_offset_31 & 2) ? ubo_load_16.zw : ubo_load_16.xy);
  vector<float16_t, 2> ubo_load_15_xz = vector<float16_t, 2>(f16tof32(ubo_load_15 & 0xFFFF));
  vector<float16_t, 2> ubo_load_15_yw = vector<float16_t, 2>(f16tof32(ubo_load_15 >> 16));
  const uint scalar_offset_32 = ((offset + 8u)) / 4;
  uint4 ubo_load_18 = ub[scalar_offset_32 / 4];
  uint2 ubo_load_17 = ((scalar_offset_32 & 2) ? ubo_load_18.zw : ubo_load_18.xy);
  vector<float16_t, 2> ubo_load_17_xz = vector<float16_t, 2>(f16tof32(ubo_load_17 & 0xFFFF));
  vector<float16_t, 2> ubo_load_17_yw = vector<float16_t, 2>(f16tof32(ubo_load_17 >> 16));
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(ubo_load_15_xz[0], ubo_load_15_yw[0], ubo_load_15_xz[1], ubo_load_15_yw[1]), vector<float16_t, 4>(ubo_load_17_xz[0], ubo_load_17_yw[0], ubo_load_17_xz[1], ubo_load_17_yw[1]));
}

matrix<float16_t, 3, 2> ub_load_28(uint offset) {
  const uint scalar_offset_33 = ((offset + 0u)) / 4;
  uint ubo_load_19 = ub[scalar_offset_33 / 4][scalar_offset_33 % 4];
  const uint scalar_offset_34 = ((offset + 4u)) / 4;
  uint ubo_load_20 = ub[scalar_offset_34 / 4][scalar_offset_34 % 4];
  const uint scalar_offset_35 = ((offset + 8u)) / 4;
  uint ubo_load_21 = ub[scalar_offset_35 / 4][scalar_offset_35 % 4];
  return matrix<float16_t, 3, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_19 & 0xFFFF)), float16_t(f16tof32(ubo_load_19 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_20 & 0xFFFF)), float16_t(f16tof32(ubo_load_20 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_21 & 0xFFFF)), float16_t(f16tof32(ubo_load_21 >> 16))));
}

matrix<float16_t, 3, 3> ub_load_29(uint offset) {
  const uint scalar_offset_36 = ((offset + 0u)) / 4;
  uint4 ubo_load_23 = ub[scalar_offset_36 / 4];
  uint2 ubo_load_22 = ((scalar_offset_36 & 2) ? ubo_load_23.zw : ubo_load_23.xy);
  vector<float16_t, 2> ubo_load_22_xz = vector<float16_t, 2>(f16tof32(ubo_load_22 & 0xFFFF));
  float16_t ubo_load_22_y = f16tof32(ubo_load_22[0] >> 16);
  const uint scalar_offset_37 = ((offset + 8u)) / 4;
  uint4 ubo_load_25 = ub[scalar_offset_37 / 4];
  uint2 ubo_load_24 = ((scalar_offset_37 & 2) ? ubo_load_25.zw : ubo_load_25.xy);
  vector<float16_t, 2> ubo_load_24_xz = vector<float16_t, 2>(f16tof32(ubo_load_24 & 0xFFFF));
  float16_t ubo_load_24_y = f16tof32(ubo_load_24[0] >> 16);
  const uint scalar_offset_38 = ((offset + 16u)) / 4;
  uint4 ubo_load_27 = ub[scalar_offset_38 / 4];
  uint2 ubo_load_26 = ((scalar_offset_38 & 2) ? ubo_load_27.zw : ubo_load_27.xy);
  vector<float16_t, 2> ubo_load_26_xz = vector<float16_t, 2>(f16tof32(ubo_load_26 & 0xFFFF));
  float16_t ubo_load_26_y = f16tof32(ubo_load_26[0] >> 16);
  return matrix<float16_t, 3, 3>(vector<float16_t, 3>(ubo_load_22_xz[0], ubo_load_22_y, ubo_load_22_xz[1]), vector<float16_t, 3>(ubo_load_24_xz[0], ubo_load_24_y, ubo_load_24_xz[1]), vector<float16_t, 3>(ubo_load_26_xz[0], ubo_load_26_y, ubo_load_26_xz[1]));
}

matrix<float16_t, 3, 4> ub_load_30(uint offset) {
  const uint scalar_offset_39 = ((offset + 0u)) / 4;
  uint4 ubo_load_29 = ub[scalar_offset_39 / 4];
  uint2 ubo_load_28 = ((scalar_offset_39 & 2) ? ubo_load_29.zw : ubo_load_29.xy);
  vector<float16_t, 2> ubo_load_28_xz = vector<float16_t, 2>(f16tof32(ubo_load_28 & 0xFFFF));
  vector<float16_t, 2> ubo_load_28_yw = vector<float16_t, 2>(f16tof32(ubo_load_28 >> 16));
  const uint scalar_offset_40 = ((offset + 8u)) / 4;
  uint4 ubo_load_31 = ub[scalar_offset_40 / 4];
  uint2 ubo_load_30 = ((scalar_offset_40 & 2) ? ubo_load_31.zw : ubo_load_31.xy);
  vector<float16_t, 2> ubo_load_30_xz = vector<float16_t, 2>(f16tof32(ubo_load_30 & 0xFFFF));
  vector<float16_t, 2> ubo_load_30_yw = vector<float16_t, 2>(f16tof32(ubo_load_30 >> 16));
  const uint scalar_offset_41 = ((offset + 16u)) / 4;
  uint4 ubo_load_33 = ub[scalar_offset_41 / 4];
  uint2 ubo_load_32 = ((scalar_offset_41 & 2) ? ubo_load_33.zw : ubo_load_33.xy);
  vector<float16_t, 2> ubo_load_32_xz = vector<float16_t, 2>(f16tof32(ubo_load_32 & 0xFFFF));
  vector<float16_t, 2> ubo_load_32_yw = vector<float16_t, 2>(f16tof32(ubo_load_32 >> 16));
  return matrix<float16_t, 3, 4>(vector<float16_t, 4>(ubo_load_28_xz[0], ubo_load_28_yw[0], ubo_load_28_xz[1], ubo_load_28_yw[1]), vector<float16_t, 4>(ubo_load_30_xz[0], ubo_load_30_yw[0], ubo_load_30_xz[1], ubo_load_30_yw[1]), vector<float16_t, 4>(ubo_load_32_xz[0], ubo_load_32_yw[0], ubo_load_32_xz[1], ubo_load_32_yw[1]));
}

matrix<float16_t, 4, 2> ub_load_31(uint offset) {
  const uint scalar_offset_42 = ((offset + 0u)) / 4;
  uint ubo_load_34 = ub[scalar_offset_42 / 4][scalar_offset_42 % 4];
  const uint scalar_offset_43 = ((offset + 4u)) / 4;
  uint ubo_load_35 = ub[scalar_offset_43 / 4][scalar_offset_43 % 4];
  const uint scalar_offset_44 = ((offset + 8u)) / 4;
  uint ubo_load_36 = ub[scalar_offset_44 / 4][scalar_offset_44 % 4];
  const uint scalar_offset_45 = ((offset + 12u)) / 4;
  uint ubo_load_37 = ub[scalar_offset_45 / 4][scalar_offset_45 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load_34 & 0xFFFF)), float16_t(f16tof32(ubo_load_34 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_35 & 0xFFFF)), float16_t(f16tof32(ubo_load_35 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_36 & 0xFFFF)), float16_t(f16tof32(ubo_load_36 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_37 & 0xFFFF)), float16_t(f16tof32(ubo_load_37 >> 16))));
}

matrix<float16_t, 4, 3> ub_load_32(uint offset) {
  const uint scalar_offset_46 = ((offset + 0u)) / 4;
  uint4 ubo_load_39 = ub[scalar_offset_46 / 4];
  uint2 ubo_load_38 = ((scalar_offset_46 & 2) ? ubo_load_39.zw : ubo_load_39.xy);
  vector<float16_t, 2> ubo_load_38_xz = vector<float16_t, 2>(f16tof32(ubo_load_38 & 0xFFFF));
  float16_t ubo_load_38_y = f16tof32(ubo_load_38[0] >> 16);
  const uint scalar_offset_47 = ((offset + 8u)) / 4;
  uint4 ubo_load_41 = ub[scalar_offset_47 / 4];
  uint2 ubo_load_40 = ((scalar_offset_47 & 2) ? ubo_load_41.zw : ubo_load_41.xy);
  vector<float16_t, 2> ubo_load_40_xz = vector<float16_t, 2>(f16tof32(ubo_load_40 & 0xFFFF));
  float16_t ubo_load_40_y = f16tof32(ubo_load_40[0] >> 16);
  const uint scalar_offset_48 = ((offset + 16u)) / 4;
  uint4 ubo_load_43 = ub[scalar_offset_48 / 4];
  uint2 ubo_load_42 = ((scalar_offset_48 & 2) ? ubo_load_43.zw : ubo_load_43.xy);
  vector<float16_t, 2> ubo_load_42_xz = vector<float16_t, 2>(f16tof32(ubo_load_42 & 0xFFFF));
  float16_t ubo_load_42_y = f16tof32(ubo_load_42[0] >> 16);
  const uint scalar_offset_49 = ((offset + 24u)) / 4;
  uint4 ubo_load_45 = ub[scalar_offset_49 / 4];
  uint2 ubo_load_44 = ((scalar_offset_49 & 2) ? ubo_load_45.zw : ubo_load_45.xy);
  vector<float16_t, 2> ubo_load_44_xz = vector<float16_t, 2>(f16tof32(ubo_load_44 & 0xFFFF));
  float16_t ubo_load_44_y = f16tof32(ubo_load_44[0] >> 16);
  return matrix<float16_t, 4, 3>(vector<float16_t, 3>(ubo_load_38_xz[0], ubo_load_38_y, ubo_load_38_xz[1]), vector<float16_t, 3>(ubo_load_40_xz[0], ubo_load_40_y, ubo_load_40_xz[1]), vector<float16_t, 3>(ubo_load_42_xz[0], ubo_load_42_y, ubo_load_42_xz[1]), vector<float16_t, 3>(ubo_load_44_xz[0], ubo_load_44_y, ubo_load_44_xz[1]));
}

matrix<float16_t, 4, 4> ub_load_33(uint offset) {
  const uint scalar_offset_50 = ((offset + 0u)) / 4;
  uint4 ubo_load_47 = ub[scalar_offset_50 / 4];
  uint2 ubo_load_46 = ((scalar_offset_50 & 2) ? ubo_load_47.zw : ubo_load_47.xy);
  vector<float16_t, 2> ubo_load_46_xz = vector<float16_t, 2>(f16tof32(ubo_load_46 & 0xFFFF));
  vector<float16_t, 2> ubo_load_46_yw = vector<float16_t, 2>(f16tof32(ubo_load_46 >> 16));
  const uint scalar_offset_51 = ((offset + 8u)) / 4;
  uint4 ubo_load_49 = ub[scalar_offset_51 / 4];
  uint2 ubo_load_48 = ((scalar_offset_51 & 2) ? ubo_load_49.zw : ubo_load_49.xy);
  vector<float16_t, 2> ubo_load_48_xz = vector<float16_t, 2>(f16tof32(ubo_load_48 & 0xFFFF));
  vector<float16_t, 2> ubo_load_48_yw = vector<float16_t, 2>(f16tof32(ubo_load_48 >> 16));
  const uint scalar_offset_52 = ((offset + 16u)) / 4;
  uint4 ubo_load_51 = ub[scalar_offset_52 / 4];
  uint2 ubo_load_50 = ((scalar_offset_52 & 2) ? ubo_load_51.zw : ubo_load_51.xy);
  vector<float16_t, 2> ubo_load_50_xz = vector<float16_t, 2>(f16tof32(ubo_load_50 & 0xFFFF));
  vector<float16_t, 2> ubo_load_50_yw = vector<float16_t, 2>(f16tof32(ubo_load_50 >> 16));
  const uint scalar_offset_53 = ((offset + 24u)) / 4;
  uint4 ubo_load_53 = ub[scalar_offset_53 / 4];
  uint2 ubo_load_52 = ((scalar_offset_53 & 2) ? ubo_load_53.zw : ubo_load_53.xy);
  vector<float16_t, 2> ubo_load_52_xz = vector<float16_t, 2>(f16tof32(ubo_load_52 & 0xFFFF));
  vector<float16_t, 2> ubo_load_52_yw = vector<float16_t, 2>(f16tof32(ubo_load_52 >> 16));
  return matrix<float16_t, 4, 4>(vector<float16_t, 4>(ubo_load_46_xz[0], ubo_load_46_yw[0], ubo_load_46_xz[1], ubo_load_46_yw[1]), vector<float16_t, 4>(ubo_load_48_xz[0], ubo_load_48_yw[0], ubo_load_48_xz[1], ubo_load_48_yw[1]), vector<float16_t, 4>(ubo_load_50_xz[0], ubo_load_50_yw[0], ubo_load_50_xz[1], ubo_load_50_yw[1]), vector<float16_t, 4>(ubo_load_52_xz[0], ubo_load_52_yw[0], ubo_load_52_xz[1], ubo_load_52_yw[1]));
}

typedef float3 ub_load_34_ret[2];
ub_load_34_ret ub_load_34(uint offset) {
  float3 arr[2] = (float3[2])0;
  {
    for(uint i = 0u; (i < 2u); i = (i + 1u)) {
      const uint scalar_offset_54 = ((offset + (i * 16u))) / 4;
      arr[i] = asfloat(ub[scalar_offset_54 / 4].xyz);
    }
  }
  return arr;
}

typedef matrix<float16_t, 4, 2> ub_load_35_ret[2];
ub_load_35_ret ub_load_35(uint offset) {
  matrix<float16_t, 4, 2> arr_1[2] = (matrix<float16_t, 4, 2>[2])0;
  {
    for(uint i_1 = 0u; (i_1 < 2u); i_1 = (i_1 + 1u)) {
      arr_1[i_1] = ub_load_31((offset + (i_1 * 16u)));
    }
  }
  return arr_1;
}

Inner ub_load_36(uint offset) {
  const uint scalar_offset_55 = ((offset + 0u)) / 4;
  const uint scalar_offset_56 = ((offset + 4u)) / 4;
  const uint scalar_offset_bytes = ((offset + 8u));
  const uint scalar_offset_index = scalar_offset_bytes / 4;
  Inner tint_symbol = {asint(ub[scalar_offset_55 / 4][scalar_offset_55 % 4]), asfloat(ub[scalar_offset_56 / 4][scalar_offset_56 % 4]), float16_t(f16tof32(((ub[scalar_offset_index / 4][scalar_offset_index % 4] >> (scalar_offset_bytes % 4 == 0 ? 0 : 16)) & 0xFFFF)))};
  return tint_symbol;
}

typedef Inner ub_load_37_ret[4];
ub_load_37_ret ub_load_37(uint offset) {
  Inner arr_2[4] = (Inner[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_2[i_2] = ub_load_36((offset + (i_2 * 16u)));
    }
  }
  return arr_2;
}

[numthreads(1, 1, 1)]
void main() {
  float scalar_f32 = asfloat(ub[0].x);
  int scalar_i32 = asint(ub[0].y);
  uint scalar_u32 = ub[0].z;
  float16_t scalar_f16 = float16_t(f16tof32(((ub[0].w) & 0xFFFF)));
  float2 vec2_f32 = asfloat(ub[1].xy);
  int2 vec2_i32 = asint(ub[1].zw);
  uint2 vec2_u32 = ub[2].xy;
  uint ubo_load_54 = ub[2].z;
  vector<float16_t, 2> vec2_f16 = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_54 & 0xFFFF)), float16_t(f16tof32(ubo_load_54 >> 16)));
  float3 vec3_f32 = asfloat(ub[3].xyz);
  int3 vec3_i32 = asint(ub[4].xyz);
  uint3 vec3_u32 = ub[5].xyz;
  uint2 ubo_load_55 = ub[6].xy;
  vector<float16_t, 2> ubo_load_55_xz = vector<float16_t, 2>(f16tof32(ubo_load_55 & 0xFFFF));
  float16_t ubo_load_55_y = f16tof32(ubo_load_55[0] >> 16);
  vector<float16_t, 3> vec3_f16 = vector<float16_t, 3>(ubo_load_55_xz[0], ubo_load_55_y, ubo_load_55_xz[1]);
  float4 vec4_f32 = asfloat(ub[7]);
  int4 vec4_i32 = asint(ub[8]);
  uint4 vec4_u32 = ub[9];
  uint2 ubo_load_56 = ub[10].xy;
  vector<float16_t, 2> ubo_load_56_xz = vector<float16_t, 2>(f16tof32(ubo_load_56 & 0xFFFF));
  vector<float16_t, 2> ubo_load_56_yw = vector<float16_t, 2>(f16tof32(ubo_load_56 >> 16));
  vector<float16_t, 4> vec4_f16 = vector<float16_t, 4>(ubo_load_56_xz[0], ubo_load_56_yw[0], ubo_load_56_xz[1], ubo_load_56_yw[1]);
  float2x2 mat2x2_f32 = ub_load_16(168u);
  float2x3 mat2x3_f32 = ub_load_17(192u);
  float2x4 mat2x4_f32 = ub_load_18(224u);
  float3x2 mat3x2_f32 = ub_load_19(256u);
  float3x3 mat3x3_f32 = ub_load_20(288u);
  float3x4 mat3x4_f32 = ub_load_21(336u);
  float4x2 mat4x2_f32 = ub_load_22(384u);
  float4x3 mat4x3_f32 = ub_load_23(416u);
  float4x4 mat4x4_f32 = ub_load_24(480u);
  matrix<float16_t, 2, 2> mat2x2_f16 = ub_load_25(544u);
  matrix<float16_t, 2, 3> mat2x3_f16 = ub_load_26(552u);
  matrix<float16_t, 2, 4> mat2x4_f16 = ub_load_27(568u);
  matrix<float16_t, 3, 2> mat3x2_f16 = ub_load_28(584u);
  matrix<float16_t, 3, 3> mat3x3_f16 = ub_load_29(600u);
  matrix<float16_t, 3, 4> mat3x4_f16 = ub_load_30(624u);
  matrix<float16_t, 4, 2> mat4x2_f16 = ub_load_31(648u);
  matrix<float16_t, 4, 3> mat4x3_f16 = ub_load_32(664u);
  matrix<float16_t, 4, 4> mat4x4_f16 = ub_load_33(696u);
  float3 arr2_vec3_f32[2] = ub_load_34(736u);
  matrix<float16_t, 4, 2> arr2_mat4x2_f16[2] = ub_load_35(768u);
  Inner struct_inner = ub_load_36(800u);
  Inner array_struct_inner[4] = ub_load_37(816u);
  s.Store(0u, asuint((((((((((((((((((((((((((((((((((((((tint_ftoi(scalar_f32) + scalar_i32) + int(scalar_u32)) + int(scalar_f16)) + tint_ftoi(vec2_f32.x)) + vec2_i32.x) + int(vec2_u32.x)) + int(vec2_f16.x)) + tint_ftoi(vec3_f32.y)) + vec3_i32.y) + int(vec3_u32.y)) + int(vec3_f16.y)) + tint_ftoi(vec4_f32.z)) + vec4_i32.z) + int(vec4_u32.z)) + int(vec4_f16.z)) + tint_ftoi(mat2x2_f32[0].x)) + tint_ftoi(mat2x3_f32[0].x)) + tint_ftoi(mat2x4_f32[0].x)) + tint_ftoi(mat3x2_f32[0].x)) + tint_ftoi(mat3x3_f32[0].x)) + tint_ftoi(mat3x4_f32[0].x)) + tint_ftoi(mat4x2_f32[0].x)) + tint_ftoi(mat4x3_f32[0].x)) + tint_ftoi(mat4x4_f32[0].x)) + int(mat2x2_f16[0].x)) + int(mat2x3_f16[0].x)) + int(mat2x4_f16[0].x)) + int(mat3x2_f16[0].x)) + int(mat3x3_f16[0].x)) + int(mat3x4_f16[0].x)) + int(mat4x2_f16[0].x)) + int(mat4x3_f16[0].x)) + int(mat4x4_f16[0].x)) + tint_ftoi(arr2_vec3_f32[0].x)) + int(arr2_mat4x2_f16[0][0].x)) + struct_inner.scalar_i32) + array_struct_inner[0].scalar_i32)));
  return;
}
