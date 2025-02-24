RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void f() {
  int c = 0;
  int d = 0;
  s.Store(0u, asuint((c + d)));
  return;
}
