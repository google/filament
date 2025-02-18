cbuffer cbuffer_u : register(b0) {
  uint4 u[8];
};
RWByteAddressBuffer s : register(u1);

void s_store_1(uint offset, float2x4 value) {
  s.Store4((offset + 0u), asuint(value[0u]));
  s.Store4((offset + 16u), asuint(value[1u]));
}

void s_store(uint offset, float2x4 value[4]) {
  float2x4 array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      s_store_1((offset + (i * 32u)), array_1[i]);
    }
  }
}

float2x4 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(u[scalar_offset / 4]), asfloat(u[scalar_offset_1 / 4]));
}

typedef float2x4 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float2x4 arr[4] = (float2x4[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s_store_1(32u, u_load_1(64u));
  s.Store4(32u, asuint(asfloat(u[1]).ywxz));
  s.Store(32u, asuint(asfloat(u[1].x)));
  return;
}
