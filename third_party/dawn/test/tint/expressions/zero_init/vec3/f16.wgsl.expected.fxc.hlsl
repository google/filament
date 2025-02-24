SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  vector<float16_t, 3> v = (float16_t(0.0h)).xxx;
}
FXC validation failure:
<scrubbed_path>(7,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
