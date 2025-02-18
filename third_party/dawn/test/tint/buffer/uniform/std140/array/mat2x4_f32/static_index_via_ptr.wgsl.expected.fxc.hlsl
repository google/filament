cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);

float2x4 a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  return float2x4(asfloat(a[scalar_offset / 4]), asfloat(a[scalar_offset_1 / 4]));
}

typedef float2x4 a_load_ret[4];
a_load_ret a_load(uint offset) {
  float2x4 arr[4] = (float2x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = a_load_1((offset + (i * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float2x4 l_a[4] = a_load(0u);
  float2x4 l_a_i = a_load_1(64u);
  float4 l_a_i_i = asfloat(a[5]);
  s.Store(0u, asuint((((asfloat(a[5].x) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x)));
  return;
}
