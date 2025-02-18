struct S {
  int i;
};


RWByteAddressBuffer s : register(u0);
S v(uint offset) {
  S v_1 = {asint(s.Load((offset + 0u)))};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  v(0u);
  asint(s.Load(0u));
}

