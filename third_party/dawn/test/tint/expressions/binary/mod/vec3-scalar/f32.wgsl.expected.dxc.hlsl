float3 tint_trunc(float3 param_0) {
  return param_0 < 0 ? ceil(param_0) : floor(param_0);
}

float3 tint_float_mod(float3 lhs, float rhs) {
  float3 r = float3((rhs).xxx);
  return (lhs - (tint_trunc((lhs / r)) * r));
}

[numthreads(1, 1, 1)]
void f() {
  float3 a = float3(1.0f, 2.0f, 3.0f);
  float b = 4.0f;
  float3 r = tint_float_mod(a, b);
  return;
}
