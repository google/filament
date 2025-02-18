
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 4> b = vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(-4.0h));
}

