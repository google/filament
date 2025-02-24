SKIP: INVALID

struct S {
  vector<float16_t, 3> v;
};


static S P = (S)0;
void f() {
  P.v = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  P.v[0u] = float16_t(1.0h);
  P.v[1u] = float16_t(2.0h);
  P.v[2u] = float16_t(3.0h);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
