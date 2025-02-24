SKIP: INVALID


void f() {
  vector<float16_t, 2> v2 = (float16_t(3.0h)).xx;
  vector<float16_t, 3> v3 = (float16_t(3.0h)).xxx;
  vector<float16_t, 4> v4 = (float16_t(3.0h)).xxxx;
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(3,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(5,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
