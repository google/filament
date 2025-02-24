RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, matrix<float16_t, 2, 2> value) {
  tint_symbol.Store<vector<float16_t, 2> >((offset + 0u), value[0u]);
  tint_symbol.Store<vector<float16_t, 2> >((offset + 4u), value[1u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 2> m = matrix<float16_t, 2, 2>((float16_t(0.0h)).xx, (float16_t(0.0h)).xx);
  tint_symbol_store(0u, matrix<float16_t, 2, 2>(m));
  return;
}
