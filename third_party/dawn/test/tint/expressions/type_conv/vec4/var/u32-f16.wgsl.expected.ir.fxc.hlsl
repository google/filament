SKIP: INVALID


static uint4 u = (1u).xxxx;
void f() {
  vector<float16_t, 4> v = vector<float16_t, 4>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(4,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
