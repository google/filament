SKIP: INVALID

struct Inner {
  matrix<float16_t, 4, 2> m;
};
struct Outer {
  Inner a[4];
};

cbuffer cbuffer_a : register(b0) {
  uint4 a[64];
};
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

matrix<float16_t, 4, 2> a_load_4(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint ubo_load = a[scalar_offset / 4][scalar_offset % 4];
  const uint scalar_offset_1 = ((offset + 4u)) / 4;
  uint ubo_load_1 = a[scalar_offset_1 / 4][scalar_offset_1 % 4];
  const uint scalar_offset_2 = ((offset + 8u)) / 4;
  uint ubo_load_2 = a[scalar_offset_2 / 4][scalar_offset_2 % 4];
  const uint scalar_offset_3 = ((offset + 12u)) / 4;
  uint ubo_load_3 = a[scalar_offset_3 / 4][scalar_offset_3 % 4];
  return matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(f16tof32(ubo_load & 0xFFFF)), float16_t(f16tof32(ubo_load >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_1 & 0xFFFF)), float16_t(f16tof32(ubo_load_1 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_2 & 0xFFFF)), float16_t(f16tof32(ubo_load_2 >> 16))), vector<float16_t, 2>(float16_t(f16tof32(ubo_load_3 & 0xFFFF)), float16_t(f16tof32(ubo_load_3 >> 16))));
}

Inner a_load_3(uint offset) {
  Inner tint_symbol_4 = {a_load_4((offset + 0u))};
  return tint_symbol_4;
}

typedef Inner a_load_2_ret[4];
a_load_2_ret a_load_2(uint offset) {
  Inner arr[4] = (Inner[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = a_load_3((offset + (i_1 * 64u)));
    }
  }
  return arr;
}

Outer a_load_1(uint offset) {
  Outer tint_symbol_5 = {a_load_2((offset + 0u))};
  return tint_symbol_5;
}

typedef Outer a_load_ret[4];
a_load_ret a_load(uint offset) {
  Outer arr_1[4] = (Outer[4])0;
  {
    for(uint i_2 = 0u; (i_2 < 4u); i_2 = (i_2 + 1u)) {
      arr_1[i_2] = a_load_1((offset + (i_2 * 256u)));
    }
  }
  return arr_1;
}

[numthreads(1, 1, 1)]
void f() {
  int p_a_i_save = i();
  int p_a_i_a_i_save = i();
  int p_a_i_a_i_m_i_save = i();
  Outer l_a[4] = a_load(0u);
  Outer l_a_i = a_load_1((256u * uint(p_a_i_save)));
  Inner l_a_i_a[4] = a_load_2((256u * uint(p_a_i_save)));
  Inner l_a_i_a_i = a_load_3(((256u * uint(p_a_i_save)) + (64u * uint(p_a_i_a_i_save))));
  matrix<float16_t, 4, 2> l_a_i_a_i_m = a_load_4(((256u * uint(p_a_i_save)) + (64u * uint(p_a_i_a_i_save))));
  const uint scalar_offset_4 = ((((256u * uint(p_a_i_save)) + (64u * uint(p_a_i_a_i_save))) + (4u * uint(p_a_i_a_i_m_i_save)))) / 4;
  uint ubo_load_4 = a[scalar_offset_4 / 4][scalar_offset_4 % 4];
  vector<float16_t, 2> l_a_i_a_i_m_i = vector<float16_t, 2>(float16_t(f16tof32(ubo_load_4 & 0xFFFF)), float16_t(f16tof32(ubo_load_4 >> 16)));
  int tint_symbol = p_a_i_save;
  int tint_symbol_1 = p_a_i_a_i_save;
  int tint_symbol_2 = p_a_i_a_i_m_i_save;
  int tint_symbol_3 = i();
  const uint scalar_offset_bytes = (((((256u * uint(tint_symbol)) + (64u * uint(tint_symbol_1))) + (4u * uint(tint_symbol_2))) + (2u * uint(tint_symbol_3))));
  const uint scalar_offset_index = scalar_offset_bytes / 4;
  float16_t l_a_i_a_i_m_i_i = float16_t(f16tof32(((a[scalar_offset_index / 4][scalar_offset_index % 4] >> (scalar_offset_bytes % 4 == 0 ? 0 : 16)) & 0xFFFF)));
  return;
}
FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
