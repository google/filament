RWByteAddressBuffer s : register(u0);

[numthreads(1, 1, 1)]
void f() {
  int a = 1;
  int a__ = a;
  int b = a;
  int b__ = a__;
  s.Store(0u, asuint((((a + a__) + b) + b__)));
  return;
}
