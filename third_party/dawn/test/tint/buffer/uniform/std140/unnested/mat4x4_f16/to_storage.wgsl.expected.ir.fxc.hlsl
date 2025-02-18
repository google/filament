SKIP: INVALID


cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);
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

void v_4(uint offset, matrix<float16_t, 4, 4> obj) {
  s.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
  s.Store<vector<float16_t, 4> >((offset + 16u), obj[2u]);
  s.Store<vector<float16_t, 4> >((offset + 24u), obj[3u]);
}

matrix<float16_t, 4, 4> v_5(uint start_byte_offset) {
  vector<float16_t, 4> v_6 = tint_bitcast_to_f16(u[(start_byte_offset / 16u)]);
  vector<float16_t, 4> v_7 = tint_bitcast_to_f16(u[((8u + start_byte_offset) / 16u)]);
  vector<float16_t, 4> v_8 = tint_bitcast_to_f16(u[((16u + start_byte_offset) / 16u)]);
  return matrix<float16_t, 4, 4>(v_6, v_7, v_8, tint_bitcast_to_f16(u[((24u + start_byte_offset) / 16u)]));
}

[numthreads(1, 1, 1)]
void f() {
  v_4(0u, v_5(0u));
  s.Store<vector<float16_t, 4> >(8u, tint_bitcast_to_f16(u[0u]));
  s.Store<vector<float16_t, 4> >(8u, tint_bitcast_to_f16(u[0u]).ywxz);
  s.Store<float16_t>(2u, float16_t(f16tof32(u[0u].z)));
}

FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
