struct Inner {
  float2x4 m;
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

float2x4 a_load_4(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(a[scalar_offset / 4]), asfloat(a[scalar_offset_1 / 4]));
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
  Outer l_a_i = a_load_1((256u * min(uint(p_a_i_save), 3u)));
  Inner l_a_i_a[4] = a_load_2((256u * min(uint(p_a_i_save), 3u)));
  Inner l_a_i_a_i = a_load_3(((256u * min(uint(p_a_i_save), 3u)) + (64u * min(uint(p_a_i_a_i_save), 3u))));
  float2x4 l_a_i_a_i_m = a_load_4(((256u * min(uint(p_a_i_save), 3u)) + (64u * min(uint(p_a_i_a_i_save), 3u))));
  const uint scalar_offset_2 = ((((256u * min(uint(p_a_i_save), 3u)) + (64u * min(uint(p_a_i_a_i_save), 3u))) + (16u * min(uint(p_a_i_a_i_m_i_save), 1u)))) / 4;
  float4 l_a_i_a_i_m_i = asfloat(a[scalar_offset_2 / 4]);
  int tint_symbol = p_a_i_save;
  int tint_symbol_1 = p_a_i_a_i_save;
  int tint_symbol_2 = p_a_i_a_i_m_i_save;
  int tint_symbol_3 = i();
  const uint scalar_offset_3 = (((((256u * min(uint(tint_symbol), 3u)) + (64u * min(uint(tint_symbol_1), 3u))) + (16u * min(uint(tint_symbol_2), 1u))) + (4u * min(uint(tint_symbol_3), 3u)))) / 4;
  float l_a_i_a_i_m_i_i = asfloat(a[scalar_offset_3 / 4][scalar_offset_3 % 4]);
  return;
}
