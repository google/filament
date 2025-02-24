cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);

void s_store_1(uint offset, float2x2 value) {
  s.Store2((offset + 0u), asuint(value[0u]));
  s.Store2((offset + 8u), asuint(value[1u]));
}

void s_store(uint offset, float2x2 value[4]) {
  float2x2 array_1[4] = value;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      s_store_1((offset + (i * 16u)), array_1[i]);
    }
  }
}

float2x2 u_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

typedef float2x2 u_load_ret[4];
u_load_ret u_load(uint offset) {
  float2x2 arr[4] = (float2x2[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = u_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  s_store(0u, u_load(0u));
  s_store_1(16u, u_load_1(32u));
  s.Store2(16u, asuint(asfloat(u[0].zw).yx));
  s.Store(16u, asuint(asfloat(u[0].z)));
  return;
}
