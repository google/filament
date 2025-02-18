
[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 2> a = vector<float16_t, 2>(float16_t(1.0h), float16_t(2.0h));
  vector<float16_t, 2> b = a;
}

