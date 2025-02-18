struct S {
  int before;
  float4x2 m;
  int after;
};

cbuffer cbuffer_u : register(b0) {
  uint4 u[32];
};
static S p[4] = (S[4])0;

float4x2 u_load_3(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  uint4 ubo_load = u[scalar_offset / 4];
  const uint scalar_offset_1 = ((offset + 8u)) / 4;
  uint4 ubo_load_1 = u[scalar_offset_1 / 4];
  const uint scalar_offset_2 = ((offset + 16u)) / 4;
  uint4 ubo_load_2 = u[scalar_offset_2 / 4];
  const uint scalar_offset_3 = ((offset + 24u)) / 4;
  uint4 ubo_load_3 = u[scalar_offset_3 / 4];
  return float4x2(asfloat(((scalar_offset & 2) ? ubo_load.zw : ubo_load.xy)), asfloat(((scalar_offset_1 & 2) ? ubo_load_1.zw : ubo_load_1.xy)), asfloat(((scalar_offset_2 & 2) ? ubo_load_2.zw : ubo_load_2.xy)), asfloat(((scalar_offset_3 & 2) ? ubo_load_3.zw : ubo_load_3.xy)));
}

S u_load_1(uint offset) {
  const uint scalar_offset_4 = ((offset + 0u)) / 4;
  const uint scalar_offset_5 = ((offset + 64u)) / 4;
  S tint_symbol = {asint(u[scalar_offset_4 / 4][scalar_offset_4 % 4]), u_load_3((offset + 8u)), asint(u[scalar_offset_5 / 4][scalar_offset_5 % 4])};
  return tint_symbol;
}

typedef S u_load_ret[4];
u_load_ret u_load(uint offset) {
  S arr[4] = (S[4])0;
  {
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 128u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  p[1] = u_load_1(256u);
  p[3].m = u_load_3(264u);
  p[1].m[0] = asfloat(u[1].xy).yx;
  return;
}
