struct Inner {
  float16_t scalar_f16;
  vector<float16_t, 3> vec3_f16;
  matrix<float16_t, 2, 4> mat2x4_f16;
};
struct S {
  Inner inner;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[2];
};
RWByteAddressBuffer s : register(u1);

matrix<float16_t, 2, 4> u_load_4(uint offset) {
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
  return matrix<float16_t, 2, 4>(vector<float16_t, 4>(ubo_load_xz[0], ubo_load_yw[0], ubo_load_xz[1], ubo_load_yw[1]), vector<float16_t, 4>(ubo_load_2_xz[0], ubo_load_2_yw[0], ubo_load_2_xz[1], ubo_load_2_yw[1]));
}

Inner u_load_1(uint offset) {
  const uint scalar_offset_bytes = ((offset + 0u));
  const uint scalar_offset_index = scalar_offset_bytes / 4;
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint4 ubo_load_5 = u[scalar_offset_2 / 4];
  uint2 ubo_load_4 = ((scalar_offset_2 & 2) ? ubo_load_5.zw : ubo_load_5.xy);
  vector<float16_t, 2> ubo_load_4_xz = vector<float16_t, 2>(f16tof32(ubo_load_4 & 0xFFFF));
  float16_t ubo_load_4_y = f16tof32(ubo_load_4[0] >> 16);
  Inner tint_symbol = {float16_t(f16tof32(((u[scalar_offset_index / 4][scalar_offset_index % 4] >> (scalar_offset_bytes % 4 == 0 ? 0 : 16)) & 0xFFFF))), vector<float16_t, 3>(ubo_load_4_xz[0], ubo_load_4_y, ubo_load_4_xz[1]), u_load_4((offset + 16u))};
  return tint_symbol;
}

S u_load(uint offset) {
  S tint_symbol_1 = {u_load_1((offset + 0u))};
  return tint_symbol_1;
}

void s_store_4(uint offset, matrix<float16_t, 2, 4> value) {
  s.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  s.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
}

void s_store_1(uint offset, Inner value) {
  s.Store<float16_t>((offset + 0u), value.scalar_f16);
  s.Store<vector<float16_t, 3> >((offset + 8u), value.vec3_f16);
  s_store_4((offset + 16u), value.mat2x4_f16);
}

void s_store(uint offset, S value) {
  s_store_1((offset + 0u), value.inner);
}

[numthreads(1, 1, 1)]
void main() {
  S x = u_load(0u);
  s_store(0u, x);
  return;
}
