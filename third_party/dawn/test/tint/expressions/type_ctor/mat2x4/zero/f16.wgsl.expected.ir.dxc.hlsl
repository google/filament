
static matrix<float16_t, 2, 4> m = matrix<float16_t, 2, 4>((float16_t(0.0h)).xxxx, (float16_t(0.0h)).xxxx);
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 2, 4> obj) {
  v.Store<vector<float16_t, 4> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 4> >((offset + 8u), obj[1u]);
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, m);
}

