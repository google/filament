RWByteAddressBuffer S : register(u0);

void func_S_X(uint pointer[1]) {
  S.Store2((8u * pointer[0]), asuint((0.0f).xx));
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol[1] = {1u};
  func_S_X(tint_symbol);
  return;
}
