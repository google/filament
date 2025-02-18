float3 tint_trunc(float3 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float3 tint_float_mod(float lhs, float3 rhs) {
  float3 l = float3((lhs).xxx);
  return (l - (tint_trunc((l / rhs)) * rhs));
}

[numthreads(1, 1, 1)]
void f() {
  float a = 4.0f;
  float3 b = float3(1.0f, 2.0f, 3.0f);
  float3 r = tint_float_mod(a, b);
  return;
}
