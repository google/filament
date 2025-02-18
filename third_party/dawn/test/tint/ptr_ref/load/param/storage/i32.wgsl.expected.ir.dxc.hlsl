
ByteAddressBuffer S : register(t0);
int func() {
  return asint(S.Load(0u));
}

[numthreads(1, 1, 1)]
void main() {
  int r = func();
}

