
RWByteAddressBuffer ssbo : register(u0);
void v_1(uint offset, float2x2 obj) {
  ssbo.Store2((offset + 0u), asuint(obj[0u]));
  ssbo.Store2((offset + 8u), asuint(obj[1u]));
}

float2x2 v_2(uint offset) {
  return float2x2(asfloat(ssbo.Load2((offset + 0u))), asfloat(ssbo.Load2((offset + 8u))));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v = v_2(0u);
  v_1(0u, v);
}

