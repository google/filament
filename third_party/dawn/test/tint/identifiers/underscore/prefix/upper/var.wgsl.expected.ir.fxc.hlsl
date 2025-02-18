
RWByteAddressBuffer s : register(u0);
static int A = int(1);
static int _A = int(2);
[numthreads(1, 1, 1)]
void f() {
  int B = A;
  int _B = _A;
  s.Store(0u, asuint((B + _B)));
}

