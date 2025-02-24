cbuffer cbuffer_a : register(b0) {
  uint4 a[16];
};
RWByteAddressBuffer s : register(u1);

float4x4 a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(a[scalar_offset / 4]), asfloat(a[scalar_offset_1 / 4]), asfloat(a[scalar_offset_2 / 4]), asfloat(a[scalar_offset_3 / 4]));
}

typedef float4x4 a_load_ret[4];
a_load_ret a_load(uint offset) {
  float4x4 arr[4] = (float4x4[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = a_load_1((offset + (i * 64u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  float4x4 l_a[4] = a_load(0u);
  float4x4 l_a_i = a_load_1(128u);
  float4 l_a_i_i = asfloat(a[9]);
  s.Store(0u, asuint((((asfloat(a[9].x) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x)));
  return;
}
