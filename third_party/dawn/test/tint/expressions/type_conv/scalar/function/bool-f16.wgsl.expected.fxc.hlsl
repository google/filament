SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static bool t = false;

bool m() {
  t = true;
  return bool(t);
}

void f() {
  bool tint_symbol = m();
  float16_t v = float16_t(tint_symbol);
}
FXC validation failure:
<scrubbed_path>(15,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(15,13): error X3000: unrecognized identifier 'v'


tint executable returned error: exit status 1
