RWByteAddressBuffer ssbo : register(u0);

float2x2 ssbo_load(uint offset) {
  return float2x2(asfloat(ssbo.Load2((offset + 0u))), asfloat(ssbo.Load2((offset + 8u))));
}

void ssbo_store(uint offset, float2x2 value) {
  ssbo.Store2((offset + 0u), asuint(value[0u]));
  ssbo.Store2((offset + 8u), asuint(value[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  float2x2 v = ssbo_load(0u);
  ssbo_store(0u, v);
  return;
}
