struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
groupshared matrix<float16_t, 4, 3> w;
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

matrix<float16_t, 4, 3> v_4(uint start_byte_offset) {
  uint4 v_5 = u[(start_byte_offset / 16u)];
  vector<float16_t, 3> v_6 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_5.zw) : (v_5.xy))).xyz;
  uint4 v_7 = u[((8u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_8 = tint_bitcast_to_f16(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_7.zw) : (v_7.xy))).xyz;
  uint4 v_9 = u[((16u + start_byte_offset) / 16u)];
  vector<float16_t, 3> v_10 = tint_bitcast_to_f16(((((((16u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_9.zw) : (v_9.xy))).xyz;
  uint4 v_11 = u[((24u + start_byte_offset) / 16u)];
  return matrix<float16_t, 4, 3>(v_6, v_8, v_10, tint_bitcast_to_f16(((((((24u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_11.zw) : (v_11.xy))).xyz);
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = matrix<float16_t, 4, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v_4(0u);
  w[1u] = tint_bitcast_to_f16(u[0u].xy).xyz;
  w[1u] = tint_bitcast_to_f16(u[0u].xy).xyz.zxy;
  w[0u].y = float16_t(f16tof32(u[0u].z));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

