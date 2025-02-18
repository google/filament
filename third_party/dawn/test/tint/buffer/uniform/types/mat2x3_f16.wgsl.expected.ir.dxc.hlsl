
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
RWByteAddressBuffer s : register(u1);
void v_1(uint offset, matrix<float16_t, 2, 3> obj) {
  s.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  s.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
}

vector<float16_t, 4> tint_bitcast_to_f16(uint2 src) {
  uint2 v = src;
  uint2 mask = (65535u).xx;
  uint2 shift = (16u).xx;
  float2 t_low = f16tof32((v & mask));
  float2 t_high = f16tof32(((v >> shift) & mask));
  float16_t v_2 = float16_t(t_low.x);
  float16_t v_3 = float16_t(t_high.x);
  float16_t v_4 = float16_t(t_low.y);
  return vector<float16_t, 4>(v_2, v_3, v_4, float16_t(t_high.y));
}

matrix<float16_t, 2, 3> v_5(uint start_byte_offset) {
  uint4 v_6 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_7 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_6.zw) : (v_6.xy))).xyz;
  uint4 v_8 = u[((8u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 3>(v_7, tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_8.zw) : (v_8.xy))).xyz);
}

[numthreads(1, 1, 1)]
void main() {
  matrix<float16_t, 2, 3> x = v_5(0u);
  v_1(0u, x);
}

