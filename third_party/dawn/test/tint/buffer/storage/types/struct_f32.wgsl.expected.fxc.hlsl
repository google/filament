struct Inner {
  float scalar_f32;
  float3 vec3_f32;
  float2x4 mat2x4_f32;
};
struct S {
  Inner inner;
};

ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

float2x4 tint_symbol_load_4(uint offset) {
  return float2x4(asfloat(tint_symbol.Load4((offset + 0u))), asfloat(tint_symbol.Load4((offset + 16u))));
}

Inner tint_symbol_load_1(uint offset) {
  Inner tint_symbol_2 = {asfloat(tint_symbol.Load((offset + 0u))), asfloat(tint_symbol.Load3((offset + 16u))), tint_symbol_load_4((offset + 32u))};
  return tint_symbol_2;
}

S tint_symbol_load(uint offset) {
  S tint_symbol_3 = {tint_symbol_load_1((offset + 0u))};
  return tint_symbol_3;
}

void tint_symbol_1_store_4(uint offset, float2x4 value) {
  tint_symbol_1.Store4((offset + 0u), asuint(value[0u]));
  tint_symbol_1.Store4((offset + 16u), asuint(value[1u]));
}

void tint_symbol_1_store_1(uint offset, Inner value) {
  tint_symbol_1.Store((offset + 0u), asuint(value.scalar_f32));
  tint_symbol_1.Store3((offset + 16u), asuint(value.vec3_f32));
  tint_symbol_1_store_4((offset + 32u), value.mat2x4_f32);
}

void tint_symbol_1_store(uint offset, S value) {
  tint_symbol_1_store_1((offset + 0u), value.inner);
}

[numthreads(1, 1, 1)]
void main() {
  S t = tint_symbol_load(0u);
  tint_symbol_1_store(0u, t);
  return;
}
