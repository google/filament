
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

vector<float16_t, 2> tint_bitcast_to_f16_1(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_4 = float16_t(t_low);
  return vector<float16_t, 2>(v_4, float16_t(t_high));
}

matrix<float16_t, 3, 2> v_5(uint start_byte_offset) {
  vector<float16_t, 2> v_6 = tint_bitcast_to_f16_1(data[(start_byte_offset / 16u)][((start_byte_offset % 16u) / 4u)]);
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16_1(data[((4u + start_byte_offset) / 16u)][(((4u + start_byte_offset) % 16u) / 4u)]);
  return matrix<float16_t, 3, 2>(v_6, v_7, tint_bitcast_to_f16_1(data[((8u + start_byte_offset) / 16u)][(((8u + start_byte_offset) % 16u) / 4u)]));
}

void main() {
  matrix<float16_t, 3, 2> v_8 = v_5(0u);
  vector<float16_t, 2> x = mul(tint_bitcast_to_f16(data[1u].xy).xyz, v_8);
}

