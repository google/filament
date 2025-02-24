cbuffer cbuffer_a : register(b0) {
  uint4 a[4];
};
RWByteAddressBuffer s : register(u1);
static int counter = 0;

int i() {
  counter = (counter + 1);
  return counter;
}

float2x2 a_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = a[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = a[scalar_offset_1 / 4];
  return float2x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)));
}

typedef float2x2 a_load_ret[4];
a_load_ret a_load(uint offset) {
  float2x2 arr[4] = (float2x2[4])0;
  {
    for(uint i_1 = 0u; (i_1 < 4u); i_1 = (i_1 + 1u)) {
      arr[i_1] = a_load_1((offset + (i_1 * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  int p_a_i_save = i();
  int p_a_i_i_save = i();
  float2x2 l_a[4] = a_load(0u);
  float2x2 l_a_i = a_load_1((16u * min(uint(p_a_i_save), 3u)));
  const uint scalar_offset_2 = (((16u * min(uint(p_a_i_save), 3u)) + (8u * min(uint(p_a_i_i_save), 1u)))) / 4;
  uint4 ubo_load_2 = a[scalar_offset_2 / 4];
  float2 l_a_i_i = asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy));
  const uint scalar_offset_3 = (((16u * min(uint(p_a_i_save), 3u)) + (8u * min(uint(p_a_i_i_save), 1u)))) / 4;
  s.Store(0u, asuint((((asfloat(a[scalar_offset_3 / 4][scalar_offset_3 % 4]) + l_a[0][0].x) + l_a_i[0].x) + l_a_i_i.x)));
  return;
}
