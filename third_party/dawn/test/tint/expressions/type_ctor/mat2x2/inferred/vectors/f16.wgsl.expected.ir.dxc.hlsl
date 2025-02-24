
static matrix<float16_t, 2, 2> m = matrix<float16_t, 2, 2>(vector<float16_t, 2>(float16_t(0.0h), float16_t(1.0h)), vector<float16_t, 2>(float16_t(2.0h), float16_t(3.0h)));
RWByteAddressBuffer v : register(u0);
void v_1(uint offset, matrix<float16_t, 2, 2> obj) {
  v.Store<vector<float16_t, 2> >((offset + 0u), obj[0u]);
  v.Store<vector<float16_t, 2> >((offset + 4u), obj[1u]);
}

[numthreads(1, 1, 1)]
void f() {
  v_1(0u, m);
}

