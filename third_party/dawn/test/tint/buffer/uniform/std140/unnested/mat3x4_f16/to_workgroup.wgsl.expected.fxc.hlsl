SKIP: INVALID

groupshared matrix<float16_t, 3, 4> w;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    w = matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  }
  GroupMemoryBarrierWithGroupSync();
}

cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

matrix<float16_t, 3, 4> u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  vector<float16_t, 2> ubo_load_yw = vector<float16_t, 2>(f16tof32(ubo_load >> 16));
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = u[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  vector<float16_t, 2> ubo_load_2_yw = vector<float16_t, 2>(f16tof32(ubo_load_2 >> 16));
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_5 = u[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  vector<float16_t, 2> ubo_load_4_yw = vector<float16_t, 2>(f16tof32(ubo_load_4 >> 16));
  return matrix<float16_t, 3, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]), vector<float16_t, 4>(ubo_load_4_xz[0], ubo_load_4_yw[0], ubo_load_4_xz[1], ubo_load_4_yw[1]));
}

void f_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = u_load(0u);
  uint2 ubo_load_6 = u[0].xy;
  vector<float16_t, 2> ubo_load_6_xz = vector<float16_t, 2>(f16tof32(ubo_load_6 & 0xFFFF));
  vector<float16_t, 2> ubo_load_6_yw = vector<float16_t, 2>(f16tof32(ubo_load_6 >> 16));
  w[1] = vector<float16_t, 4>(ubo_load_6_xz[0], ubo_load_6_yw[0], ubo_load_6_xz[1], ubo_load_6_yw[1]);
  uint2 ubo_load_7 = u[0].xy;
  vector<float16_t, 2> ubo_load_7_xz = vector<float16_t, 2>(f16tof32(ubo_load_7 & 0xFFFF));
  vector<float16_t, 2> ubo_load_7_yw = vector<float16_t, 2>(f16tof32(ubo_load_7 >> 16));
  w[1] = vector<float16_t, 4>(ubo_load_7_xz[0], ubo_load_7_yw[0], ubo_load_7_xz[1], ubo_load_7_yw[1]).ywxz;
  w[0][1] = float16_t(f16tof32(((u[0].z) & 0xFFFF)));
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
FXC validation failure:
<scrubbed_path>(1,20-28): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
