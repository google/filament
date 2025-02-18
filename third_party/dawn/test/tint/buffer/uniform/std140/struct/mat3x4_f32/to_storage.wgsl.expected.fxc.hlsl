struct S {
  int before;
  float3x4 m;
  int after;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
RWByteAddressBuffer s : register(u1);

void s_store_3(uint offset, float3x4 value) {
  s.Store4((offset + 0u), asuint(value[0u]));
  s.Store4((offset + 16u), asuint(value[1u]));
  s.Store4((offset + 32u), asuint(value[2u]));
}

void s_store_1(uint offset, S value) {
  s.Store((offset + 0u), asuint(value.before));
  s_store_3((offset + 16u), value.m);
  s.Store((offset + 64u), asuint(value.after));
}

void s_store(uint offset, S value[4]) {
  S array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      s_store_1((offset + (i * 128u)), array_1[i]);
    }
  }
}

float3x4 u_load_3(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  return float3x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]), asfloat(u[scalar_offset_2 / 4]));
}

S u_load_1(uint offset) {
  const uint scalar_offset_3 = ((offset + 0u)) / 4;
  const uint scalar_offset_4 = ((offset + 64u)) / 4;
  S tint_symbol = {asint(u[scalar_offset_3 / 4][scalar_offset_3 % 4]), u_load_3((offset + 16u)), asint(u[scalar_offset_4 / 4][scalar_offset_4 % 4])};
  return tint_symbol;
}

typedef S u_load_ret[4];
u_load_ret u_load(uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 128u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s_store_1(128u, u_load_1(256u));
  s_store_3(400u, u_load_3(272u));
  s.Store4(144u, asuint(asfloat(u[2]).ywxz));
  return;
}
