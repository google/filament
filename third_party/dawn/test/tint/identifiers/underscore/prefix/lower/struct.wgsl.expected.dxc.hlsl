RWByteAddressBuffer s : register(u0);

struct _a {
  int _b;
};

[numthreads(1, 1, 1)]
void f() {
  _a c = (_a)0;
  int d = c._b;
  s.Store(0u, asuint((c._b + d)));
  return;
}
