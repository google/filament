
[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(4.0h);
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> v = (a / b);
  vector<float16_t, 3> r = (a - ((((v < (float16_t(0.0h)).xxx)) ? (ceil(v)) : (floor(v))) * b));
}

