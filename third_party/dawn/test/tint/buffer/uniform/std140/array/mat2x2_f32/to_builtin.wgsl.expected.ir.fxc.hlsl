
cbuffer cbuffer_u : register(b0) {
  uint4 u[4];
};
RWByteAddressBuffer s : register(u1);
float2x2 v(uint start_byte_offset) {
  uint4 v_1 = u[(start_byte_offset / 16u)];
  float2 v_2 = asfloat((((((start_byte_offset % 16u) / 4u) == 2u)) ? (v_1.zw) : (v_1.xy)));
  uint4 v_3 = u[((8u + start_byte_offset) / 16u)];
  return float2x2(v_2, asfloat(((((((8u + start_byte_offset) % 16u) / 4u) == 2u)) ? (v_3.zw) : (v_3.xy))));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 t = transpose(v(32u));
  float l = length(asfloat(u[0u].zw).yx);
  float a = abs(asfloat(u[0u].zw).yx.x);
  float v_4 = (t[0u].x + float(l));
  s.Store(0u, asuint((v_4 + float(a))));
}

