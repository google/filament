ByteAddressBuffer S : register(t0);

float4 func_S() {
  return asfloat(S.Load4(0u));
}

[numthreads(1, 1, 1)]
void main() {
  float4 r = func_S();
  return;
}
