RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float4x2 value) {
  tint_symbol.Store2((offset + 0u), asuint(value[0u]));
  tint_symbol.Store2((offset + 8u), asuint(value[1u]));
  tint_symbol.Store2((offset + 16u), asuint(value[2u]));
  tint_symbol.Store2((offset + 24u), asuint(value[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, float4x2(float2(0.0f, 1.0f), float2(2.0f, 3.0f), float2(4.0f, 5.0f), float2(6.0f, 7.0f)));
  return;
}
