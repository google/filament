struct S {
  float f;
  uint u;
  float4 v;
};

RWByteAddressBuffer output : register(u0);

void output_store(uint offset, S value) {
  output.Store((offset + 0u), asuint(value.f));
  output.Store((offset + 4u), asuint(value.u));
  output.Store4((offset + 128u), asuint(value.v));
}

void frag_main() {
  S tint_symbol = {1.0f, 2u, (3.0f).xxxx};
  output_store(0u, tint_symbol);
  return;
}
