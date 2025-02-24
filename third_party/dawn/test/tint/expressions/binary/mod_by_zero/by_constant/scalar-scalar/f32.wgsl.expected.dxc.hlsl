float tint_trunc(float param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float tint_float_mod(float lhs, float rhs) {
  return (lhs - (tint_trunc((lhs / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  float a = 1.0f;
  float b = 0.0f;
  float r = tint_float_mod(a, b);
  return;
}
