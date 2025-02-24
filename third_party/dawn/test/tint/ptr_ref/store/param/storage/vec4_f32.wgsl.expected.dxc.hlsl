RWByteAddressBuffer S : register(u0);

void func_S() {
  S.Store4(0u, asuint((0.0f).xxxx));
}

[numthreads(1, 1, 1)]
void main() {
  func_S();
  return;
}
