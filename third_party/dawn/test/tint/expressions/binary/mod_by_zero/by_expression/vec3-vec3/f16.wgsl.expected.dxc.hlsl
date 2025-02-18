vector<float16_t, 3> tint_trunc(vector<float16_t, 3> param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

vector<float16_t, 3> tint_float_mod(vector<float16_t, 3> lhs, vector<float16_t, 3> rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  vector<float16_t, 3> a = vector<float16_t, 3>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h));
  vector<float16_t, 3> b = vector<float16_t, 3>(float16_t(0.0h), float16_t(5.0h), float16_t(0.0h));
  vector<float16_t, 3> r = tint_float_mod(a, (b + b));
  return;
}
