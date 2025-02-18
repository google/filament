RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, matrix<float16_t, 3, 4> value) {
  tint_symbol.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  tint_symbol.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  tint_symbol.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
}

[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 3, 4> m = matrix<float16_t, 3, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
  tint_symbol_store(0u, matrix<float16_t, 3, 4>(m));
  return;
}
