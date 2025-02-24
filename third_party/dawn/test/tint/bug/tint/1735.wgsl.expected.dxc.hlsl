struct S {
  float f;
};

ByteAddressBuffer tint_symbol : register(t0);
RWByteAddressBuffer tint_symbol_1 : register(u1);

void tint_symbol_1_store(uint offset, S value) {
  tint_symbol_1.Store((offset + 0u), asuint(value.f));
}

S tint_symbol_load(uint offset) {
  S tint_symbol_2 = {asfloat(tint_symbol.Load((offset + 0u)))};
  return tint_symbol_2;
}

[numthreads(1, 1, 1)]
void main() {
  tint_symbol_1_store(0u, tint_symbol_load(0u));
  return;
}
