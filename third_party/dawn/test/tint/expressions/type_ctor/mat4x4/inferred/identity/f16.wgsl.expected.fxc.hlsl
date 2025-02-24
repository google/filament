SKIP: INVALID

static matrix<float16_t, 4, 4> m = matrix<float16_t, 4, 4>(vector<float16_t, 4>(float16_t(0.0h), float16_t(1.0h), float16_t(2.0h), float16_t(3.0h)), vector<float16_t, 4>(float16_t(4.0h), float16_t(5.0h), float16_t(6.0h), float16_t(7.0h)), vector<float16_t, 4>(float16_t(8.0h), float16_t(9.0h), float16_t(10.0h), float16_t(11.0h)), vector<float16_t, 4>(float16_t(12.0h), float16_t(13.0h), float16_t(14.0h), float16_t(15.0h)));
RWByteAddressBuffer tint_symbol : register(u0);

void tint_symbol_store(uint offset, matrix<float16_t, 4, 4> value) {
  tint_symbol.Store<vector<float16_t, 4> >((offset + 0u), value[0u]);
  tint_symbol.Store<vector<float16_t, 4> >((offset + 8u), value[1u]);
  tint_symbol.Store<vector<float16_t, 4> >((offset + 16u), value[2u]);
  tint_symbol.Store<vector<float16_t, 4> >((offset + 24u), value[3u]);
}

[numthreads(1, 1, 1)]
void f() {
  tint_symbol_store(0u, matrix<float16_t, 4, 4>(m));
  return;
}
FXC validation failure:
<scrubbed_path>(1,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
