cbuffer cbuffer_a : register(b0) {
  uint4 a[8];
};
RWByteAddressBuffer s : register(u1);
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

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
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = a_load_1((offset + (i_1 * 32u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  int p_a_i_save = i();
  int p_a_i_i_save = i();
  float4x2 l_a[4] = a_load(0u);
  float4x2 l_a_i = a_load_1((32u * min(uint(p_a_i_save), 3u)));
  const uint scalar_offset_4 = (((32u * min(uint(p_a_i_save), 3u)) + (8u * min(uint(p_a_i_i_save), 3u)))) / 4;
  uint4 ubo_load_4 = a[scalar_offset_4 / 4];
  float2 l_a_i_i = asfloat(((scalar_offset_4 & 2) ? ubo_load_4.zw : ubo_load_4.xy));
  const uint scalar_offset_5 = (((32u * min(uint(p_a_i_save), 3u)) + (8u * min(uint(p_a_i_i_save), 3u)))) / 4;
  s.Store(0u, asuint((((asfloat(a[scalar_offset_5 / 4][scalar_offset_5 % 4]) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x)));
  return;
}
