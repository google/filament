
RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  int a = int(1);
  int _a = a;
  int b = a;
  int _b = _a;
  s.Store(0u, asuint((((a + _a) + b) + _b)));
}

