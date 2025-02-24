SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float16_t t = float16_t(0.0h);

float16_t m() {
  t = float16_t(1.0h);
  return float16_t(t);
}

void f() {
  float16_t tint_symbol = m();
  bool v = bool(tint_symbol);
}
FXC validation failure:
<scrubbed_path>(6,8-16): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
