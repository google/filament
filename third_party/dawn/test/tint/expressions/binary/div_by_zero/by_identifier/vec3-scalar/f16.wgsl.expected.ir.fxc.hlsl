SKIP: INVALID


[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  float16_t b = float16_t(0.0h);
  vector<float16_t, 3> r = (a / b);
}

FXC validation failure:
<scrubbed_path>(4,10-18): error X3000: syntax error: unexpected token 'float16_t'
<scrubbed_path>(5,3-11): error X3000: unrecognized identifier 'float16_t'
<scrubbed_path>(5,13): error X3000: unrecognized identifier 'b'


tint executable returned error: exit status 1
