RWByteAddressBuffer S : register(u0);

void func_S_i() {
  S.Store(0u, asuint(42));
}

[numthreads(1, 1, 1)]
void main() {
  func_S_i();
  return;
}
