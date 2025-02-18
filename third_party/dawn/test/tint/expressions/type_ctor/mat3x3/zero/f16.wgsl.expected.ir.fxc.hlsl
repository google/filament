SKIP: INVALID


static matrix<float16_t, 3, 3> m = matrix<float16_t, 3, 3>((float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx, (float16_t(0.0h)).xxx);
RWByteAddressBuffer tint_symbol : register(u0);
void v(uint offset, matrix<float16_t, 3, 3> obj) {
  tint_symbol.Store<vector<float16_t, 3> >((offset + 0u), obj[0u]);
  tint_symbol.Store<vector<float16_t, 3> >((offset + 8u), obj[1u]);
  tint_symbol.Store<vector<float16_t, 3> >((offset + 16u), obj[2u]);
}

[numthreads(1, 1, 1)]
void f() {
  v(0u, m);
}

FXC validation failure:
<scrubbed_path>(2,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
