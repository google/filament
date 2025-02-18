
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
static float2x2 p = float2x2((0.0f).xx, (0.0f).xx);
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = u[((8u + start_byte_offset) / 16u)];
  return float2x2(v_2, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  p = v(0u);
  p[1u] = asfloat(u[0u].xy);
  p[1u] = asfloat(u[0u].xy).yx;
  p[0u].y = asfloat(u[0u].z);
}

