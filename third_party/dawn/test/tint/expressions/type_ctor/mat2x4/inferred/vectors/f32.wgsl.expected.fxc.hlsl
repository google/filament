static float2x4 m = float2x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f));
RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float2x4 value) {
  tint_symbol.Store4((offset + 0u), asuint(value[0u]));
  tint_symbol.Store4((offset + 16u), asuint(value[1u]));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, m);
  return;
}
