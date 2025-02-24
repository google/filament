
RWByteAddressBuffer s : register(u0);
static int a = int(1);
static int a__ = int(2);
[numthreads(1, 1, 1)]
void f() {
  int b = a;
  int b__ = a__;
  s.Store(0u, asuint((b + b__)));
}

