SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> a = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
  vector<float16_t, 4> b = a;
}

FXC validation failure:
<scrubbed_path>(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(5,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
