
cbuffer cbuffer_u : register(b0) {
  uint4 u[1];
};
void a(float2x2 m) {
}

void b(float2 v) {
}

void c(float f_1) {
}

float2x2 v_1(uint start_byte_offset) {
  uint4 v_2 = u[(start_byte_offset / 16u)];
  float2 v_3 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_2.zw) : (v_2.xy)));
  uint4 v_4 = u[((8u + start_byte_offset) / 16u)];
  return float2x2(v_3, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_4.zw) : (v_4.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  a(v_1(0u));
  b(asfloat(u[0u].zw));
  b(asfloat(u[0u].zw).yx);
  c(asfloat(u[0u].z));
  c(asfloat(u[0u].zw).yx.x);
}

