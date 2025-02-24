
RWByteAddressBuffer s : register(u0);
void f(int _a) {
  int b = _a;
  s.Store(0u, asuint(b));
}

[numthreads(1, 1, 1)]
void main() {
  f(int(1));
}

