SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t t = float16_t(0.0h);

vector<float16_t, 2> m() {
  t = float16_t(1.0h);
  return vector<float16_t, 2>((t).xx);
}

void f() {
  vector<float16_t, 2> tint_symbol = m();
  bool2 v = bool2(tint_symbol);
}
FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
