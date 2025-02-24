static float2x2 m = float2x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f));
RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float2x2 value) {
  tint_symbol.Store2((offset + 0u), asuint(value[0u]));
  tint_symbol.Store2((offset + 8u), asuint(value[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, m);
  return;
}
