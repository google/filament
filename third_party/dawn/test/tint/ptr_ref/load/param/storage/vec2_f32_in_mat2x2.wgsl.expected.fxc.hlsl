ByteAddressBuffer S : register(t0);

float2 func_S_X(uint pointer[1]) {
  return asfloat(S.Load2((8u * pointer[0])));
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol[1] = {1u};
  float2 r = func_S_X(tint_symbol);
  return;
}
