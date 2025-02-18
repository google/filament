SKIP: INVALID


static float u = 1.0f;
void f() {
  float16_t v = float16_t(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(4,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(4,13): error X3000: unrecognized identifier 'v'


tint executable returned error: exit status 1
