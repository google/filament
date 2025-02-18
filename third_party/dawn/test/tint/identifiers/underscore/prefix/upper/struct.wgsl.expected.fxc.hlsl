RWByteAddressBuffer s : register(u0);

struct _A {
  int _B;
};

[numthreads(1, 1, 1)]
void f() {
  _A c = (_A)0;
  int d = c._B;
  s.Store(0u, asuint((c._B + d)));
  return;
}
