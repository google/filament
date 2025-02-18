SKIP: INVALID

[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = a;
  return;
}
FXC validation failure:
<scrubbed_path>(3,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(3,13): error X3000: unrecognized identifier 'a'


tint executable returned error: exit status 1
