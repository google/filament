SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void f() {
  float16_t v = float16_t(0.0h);
}
FXC validation failure:
<scrubbed_path>(7,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(7,13): error X3000: unrecognized identifier 'v'


tint executable returned error: exit status 1
