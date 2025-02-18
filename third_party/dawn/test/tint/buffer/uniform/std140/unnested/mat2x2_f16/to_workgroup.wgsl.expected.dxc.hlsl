groupshared matrix<float16_t, 2, 2> w;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    w = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  }
  GroupMemoryBarrierWithGroupSync();
}

cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

matrix<float16_t, 2, 2> u_load(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = u[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = u[scalar_offset_1 / 4][scalar_offset_1 % 4];
  return matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))));
}

void f_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  w = u_load(0u);
  uint ubo_load_2 = u[0].x;
  w[1] = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16)));
  uint ubo_load_3 = u[0].x;
  w[1] = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))).yx;
  w[0][1] = float16_t(f16tof32(((u[0].y) & 0xFFFF)));
}

[numthreads(1, 1, 1)]
void f(tint_symbol_1 tint_symbol) {
  f_inner(tint_symbol.local_invocation_index);
  return;
}
