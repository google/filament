RWByteAddressBuffer s : register(u0);

void f(int a__) {
  int b = a__;
  s.Store(0u, asuint(b));
}

[numthreads(1, 1, 1)]
void main() {
  f(1);
  return;
}
