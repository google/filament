cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
static float2x2 p[4] = (float2x2[4])0;

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
    for(uint i = 0u; (i < 4u); i = (i + 1u)) {
      arr[i] = u_load_1((offset + (i * 16u)));
    }
  }
  return arr;
}

[numthreads(1, 1, 1)]
void f() {
  p = u_load(0u);
  p[1] = u_load_1(32u);
  p[1][0] = asfloat(u[0].zw).yx;
  p[1][0].x = asfloat(u[0].z);
  s.Store(0u, asuint(p[1][0].x));
  return;
}
