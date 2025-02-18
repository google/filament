RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float3x3 value) {
  tint_symbol.Store3((offset + 0u), asuint(value[0u]));
  tint_symbol.Store3((offset + 16u), asuint(value[1u]));
  tint_symbol.Store3((offset + 32u), asuint(value[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  float3x3 m = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  tint_symbol_store(0u, float3x3(m));
  return;
}
