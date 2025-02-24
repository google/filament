RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void f() {
  s.Store(0u, asuint(3));
  return;
}
