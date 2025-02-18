SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static int t = 0;

int2 m() {
  t = 1;
  return int2((t).xx);
}

void f() {
  int2 tint_symbol = m();
  vector<float16_t, 2> v = vector<float16_t, 2>(tint_symbol);
}
FXC validation failure:
<scrubbed_path>(15,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
