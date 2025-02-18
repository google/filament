struct S1 {
  int i;
};
struct S2 {
  S1 s1;
};
struct S3 {
  S2 s2;
};

int f(S3 s3) {
  return s3.s2.s1.i;
}

RWByteAddressBuffer tint_symbol : register(u0);

[numthreads(1, 1, 1)]
void main() {
  S3 tint_symbol_1 = {{{42}}};
  tint_symbol.Store(0u, asuint(f(tint_symbol_1)));
  return;
}
