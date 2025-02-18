
[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(0.0h);
  float16_t r = (a / b);
}

