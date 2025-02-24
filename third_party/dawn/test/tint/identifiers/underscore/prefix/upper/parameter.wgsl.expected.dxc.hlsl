RWByteAddressBuffer s : register(u0);

void f(int _A) {
  int B = _A;
  s.Store(0u, asuint(B));
}

[numthreads(1, 1, 1)]
void main() {
  f(1);
  return;
}
