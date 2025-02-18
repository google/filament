SKIP: INVALID


static float16_t u = float16_t(1.0h);
void f() {
  float v = float(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,8-16): error X3000: unrecognized identifier 'float16_t'


tint executable returned error: exit status 1
