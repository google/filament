SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float t = 0.0f;

float m() {
  t = 1.0f;
  return float(t);
}

void f() {
  float tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
FXC validation failure:
<scrubbed_path>(15,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(15,13): error X3000: unrecognized identifier 'v'


tint executable returned error: exit status 1
