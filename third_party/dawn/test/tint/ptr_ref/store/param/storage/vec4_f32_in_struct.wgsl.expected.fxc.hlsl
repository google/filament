RWByteAddressBuffer S : register(u0);

void func_S_i() {
  S.Store4(0u, asuint((0.0f).xxxx));
}

[numthreads(1, 1, 1)]
void main() {
  func_S_i();
  return;
}
