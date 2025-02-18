cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);

float4x2 a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = a[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = a[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = a[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = a[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
}

typedef float4x2 a_load_ret[4];
a_load_ret a_load(uint offset) {
  float4x2 arr[4] = (float4x2[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = a_load_1((offset + (i * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float4x2 l_a[4] = a_load(0u);
  float4x2 l_a_i = a_load_1(64u);
  float2 l_a_i_i = asfloat(a[4].zw);
  s.Store(0u, asuint((((asfloat(a[4].z) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x)));
  return;
}
