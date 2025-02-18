
cbuffer cbuffer_data : register(b0) {
  uint4 data[2];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_1 = float16_t(t_low.x);
  float16_t v_2 = float16_t(t_high.x);
  float16_t v_3 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_1, v_2, v_3, float16_t(t_high.y));
}

matrix<float16_t, 3, 3> v_4(uint start_byte_offset) {
  uint4 v_5 = data[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))).xyz;
  uint4 v_7 = data[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz;
  uint4 v_9 = data[((16u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 3>(v_6, v_8, tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy))).xyz);
}

void main() {
  matrix<float16_t, 3, 3> v_10 = v_4(0u);
  vector<float16_t, 3> x = mul(tint_bitcast_to_f16(data[1u].zw).xyz, v_10);
}

