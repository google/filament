groupshared matrix<float16_t, 2, 3> w[4];

void tint_zero_workgroup_memory(uint local_idx) {
  {
    for(uint idx = local_idx; (idx < 4u); idx = (idx + 1u)) {
      uint i = idx;
      w[i] = matrix<float16_t, 2, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
    }
  }
  GroupMemoryBarrierWithGroupSync();
}

cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

matrix<float16_t, 2, 3> u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset / 4];
  uint2 ubo_load = ((scalar_offset & 2) ? ubo_load_1.zw : ubo_load_1.xy);
  vector<float16_t, 2> ubo_load_xz = vector<float16_t, 2>(f16tof32(ubo_load & 0xFFFF));
  float16_t ubo_load_y = f16tof32(ubo_load[0] >> 16);
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_3 = u[scalar_offset_1 / 4];
  uint2 ubo_load_2 = ((scalar_offset_1 & 2) ? ubo_load_3.zw : ubo_load_3.xy);
  vector<float16_t, 2> ubo_load_2_xz = vector<float16_t, 2>(f16tof32(ubo_load_2 & 0xFFFF));
  float16_t ubo_load_2_y = f16tof32(ubo_load_2[0] >> 16);
  return matrix<float16_t, 2, 3>(vector<float16_t, 3>(ubo_load_xz[0], ubo_load_y, ubo_load_xz[1]), vector<float16_t, 3>(ubo_load_2_xz[0], ubo_load_2_y, ubo_load_2_xz[1]));
}

typedef matrix<float16_t, 2, 3> u_load_ret[4];
u_load_ret u_load(uint offset) {
  matrix<float16_t, 2, 3> arr[4] = (matrix<float16_t, 2, 3>[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

void f_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = u_load(0u);
  w[1] = u_load_1(32u);
  uint2 ubo_load_4 = u[0].zw;
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  w[1][0] = vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]).zxy;
  w[1][0].x = float16_t(f16tof32(((u[0].z) & 0xFFFF)));
  s.Store<float16_t>(0u, w[1][0].x);
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
