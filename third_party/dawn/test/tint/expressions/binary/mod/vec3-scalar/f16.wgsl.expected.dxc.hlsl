vector<float16_t, 3> tint_trunc(vector<float16_t, 3> param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

vector<float16_t, 3> tint_float_mod(vector<float16_t, 3> lhs, float16_t rhs) {
  vector<float16_t, 3> r = vector<float16_t, 3>((rhs).xxx);
  return (lhs - (tint_trunc((lhs / r)) * r));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  float16_t b = float16_t(4.0h);
  vector<float16_t, 3> r = tint_float_mod(a, b);
  return;
}
