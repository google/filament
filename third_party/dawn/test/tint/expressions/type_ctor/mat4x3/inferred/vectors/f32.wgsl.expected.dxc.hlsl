static float4x3 m = float4x3(float3(0.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 5.0f), float3(6.0f, 7.0f, 8.0f), float3(9.0f, 10.0f, 11.0f));
RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, float4x3 value) {
  tint_symbol.Store3((offset + 0u), asuint(value[0u]));
  tint_symbol.Store3((offset + 16u), asuint(value[1u]));
  tint_symbol.Store3((offset + 32u), asuint(value[2u]));
  tint_symbol.Store3((offset + 48u), asuint(value[3u]));
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, m);
  return;
}
