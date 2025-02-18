struct _a {
  int _b;
};


RWByteAddressBuffer s : register(u0);
[numthreads(1, 1, 1)]
void f() {
  _a c = (_a)0;
  int d = c._b;
  s.Store(0u, asuint((c._b + d)));
}

