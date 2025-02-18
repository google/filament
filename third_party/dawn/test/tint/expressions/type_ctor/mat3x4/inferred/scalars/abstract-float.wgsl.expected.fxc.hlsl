RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float3x4 value) {
  tint_symbol.Store4((offset + 0u), asuint(value[0u]));
  tint_symbol.Store4((offset + 16u), asuint(value[1u]));
  tint_symbol.Store4((offset + 32u), asuint(value[2u]));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, float3x4(float4(0.0f, 1.0f, 2.0f, 3.0f), float4(4.0f, 5.0f, 6.0f, 7.0f), float4(8.0f, 9.0f, 10.0f, 11.0f)));
  return;
}
