SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(2.0h);
  float16_t r = (a + b);
}

FXC validation failure:
<scrubbed_path>(4,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(4,13): error X3000: unrecognized identifier 'a'


tint executable returned error: exit status 1
