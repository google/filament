float16_t tint_trunc(float16_t param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float16_t tint_float_mod(float16_t lhs, float16_t rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  float16_t a = float16_t(1.0h);
  float16_t b = float16_t(0.0h);
  float16_t r = tint_float_mod(a, b);
  return;
}
