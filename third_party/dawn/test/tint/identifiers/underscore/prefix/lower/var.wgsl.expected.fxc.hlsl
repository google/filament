RWByteAddressBuffer s : register(u0);
static int a = 1;
static int _a = 2;

[numthreads(1, 1, 1)]
void f() {
  int b = a;
  int _b = _a;
  s.Store(0u, asuint((b + _b)));
  return;
}
