SKIP: INVALID

struct f_inputs {
  uint tint_local_index : SV_GroupIndex;
};


cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
groupshared matrix<float16_t, 2, 2> w;
vector<float16_t, 2> tint_bitcast_to_f16(uint src) {
  uint v = src;
  float t_low = f16tof32((v & 65535u));
  float t_high = f16tof32(((v >> 16u) & 65535u));
  float16_t v_1 = float16_t(t_low);
  return vector<float16_t, 2>(v_1, float16_t(t_high));
}

matrix<float16_t, 2, 2> v_2(uint start_byte_offset) {
  uint4 v_3 = u[(start_byte_offset / 16u)];
  vector<float16_t, 2> v_4 = tint_bitcast_to_f16((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_3.z) : (v_3.x)));
  uint4 v_5 = u[((4u + start_byte_offset) / 16u)];
  return matrix<float16_t, 2, 2>(v_4, tint_bitcast_to_f16(((((((4u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_5.z) : (v_5.x))));
}

void f_inner(uint tint_local_index) {
  if ((tint_local_index == 0u)) {
    w = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  }
  GroupMemoryBarrierWithGroupSync();
  w = v_2(0u);
  w[int(1)] = tint_bitcast_to_f16(u[0u].x);
  w[int(1)] = tint_bitcast_to_f16(u[0u].x).yx;
  w[int(0)][int(1)] = float16_t(f16tof32(u[0u].y));
}

[numthreads(1, 1, 1)]
void f(f_inputs inputs) {
  f_inner(inputs.tint_local_index);
}

FXC validation failure:
<scrubbed_path>(9,20-28): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
