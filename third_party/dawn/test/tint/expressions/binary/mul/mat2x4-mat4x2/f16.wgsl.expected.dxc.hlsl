[numthreads(1, 1, 1)]
void f() {
  matrix<float16_t, 2, 4> a = matrix<float16_t, 2, 4>(vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(4.0h)), vector<float16_t, 4>(float16_t(5.0h), float16_t(6.0h), float16_t(7.0h), float16_t(8.0h)));
  matrix<float16_t, 4, 2> b = matrix<float16_t, 4, 2>(vector<float16_t, 2>(float16_t(-1.0h), float16_t(-2.0h)), vector<float16_t, 2>(float16_t(-3.0h), float16_t(-4.0h)), vector<float16_t, 2>(float16_t(-5.0h), float16_t(-6.0h)), vector<float16_t, 2>(float16_t(-7.0h), float16_t(-8.0h)));
  matrix<float16_t, 4, 4> r = mul(b, a);
  return;
}
