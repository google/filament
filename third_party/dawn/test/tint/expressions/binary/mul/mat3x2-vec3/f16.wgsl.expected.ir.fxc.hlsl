SKIP: INVALID


cbuffer cbuffer_data : register(b0) {
  uint4 data[2];
};
vector<float16_t, 4> tint_bitcast_to_f16(uint4 src) {
  uint4 v = src;
  uint4 mask = (65535u).xxxx;
  uint4 shift = (16u).xxxx;
  float4 t_low = f16tof32((v & mask));
  float4 t_high = f16tof32(((v >> shift) & mask));
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
  uint4 v_6 = data[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_7 = tint_bitcast_to_f16_1((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.z) : (v_6.x)));
  uint4 v_8 = data[((4u + start_byte_offset) / 16u)];
  vector<float16_t, 2> v_9 = tint_bitcast_to_f16_1(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.z) : (v_8.x)));
  uint4 v_10 = data[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 3, 2>(v_7, v_9, tint_bitcast_to_f16_1(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_10.z) : (v_10.x))));
}

void main() {
  matrix<float16_t, 3, 2> v_11 = v_5(0u);
  vector<float16_t, 2> x = mul(tint_bitcast_to_f16(data[1u]).xyz, v_11);
}

FXC validation failure:
<scrubbed_path>(5,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
