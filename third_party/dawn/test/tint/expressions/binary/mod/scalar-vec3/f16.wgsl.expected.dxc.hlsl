vector<float16_t, 3> tint_trunc(vector<float16_t, 3> param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

vector<float16_t, 3> tint_float_mod(float16_t lhs, vector<float16_t, 3> rhs) {
  vector<float16_t, 3> l = vector<float16_t, 3>((lhs).xxx);
  return (l - (tint_trunc((l / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(4.0h);
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> r = tint_float_mod(a, b);
  return;
}
