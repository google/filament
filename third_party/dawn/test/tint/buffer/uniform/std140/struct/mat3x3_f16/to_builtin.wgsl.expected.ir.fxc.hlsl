SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
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

matrix<float16_t, 3, 3> v_4(uint start_byte_offset) {
  vector<float16_t, 3> v_5 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]).xyz;
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]).xyz;
  return matrix<float16_t, 3, 3>(v_5, v_6, tint_bitcast_to_f16(u[((16u + start_byte_offset) / 16u)]).xyz);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 3, 3> t = transpose(v_4(264u));
  float16_t l = length(tint_bitcast_to_f16(u[1u]).xyz.zxy);
  float16_t a = abs(tint_bitcast_to_f16(u[1u]).xyz.zxy[0u]);
}

FXC validation failure:
<scrubbed_path>(5,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
