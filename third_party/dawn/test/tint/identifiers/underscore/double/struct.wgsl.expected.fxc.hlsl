RWByteAddressBuffer s : register(u0);

struct a__ {
  int b__;
};

[numthreads(1, 1, 1)]
void f() {
  a__ c = (a__)0;
  int d = c.b__;
  s.Store(0u, asuint((c.b__ + d)));
  return;
}
