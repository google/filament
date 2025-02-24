ByteAddressBuffer S : register(t0);

int func_S() {
  return asint(S.Load(0u));
}

[numthreads(1, 1, 1)]
void main() {
  int r = func_S();
  return;
}
