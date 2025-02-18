struct S {
  float f;
  uint u;
  float4 v;
};


RWByteAddressBuffer output : register(u0);
void v_1(uint offset, S obj) {
  output.Store((offset + 0u), asuint(obj.f));
  output.Store((offset + 4u), obj.u);
  output.Store4((offset + 128u), asuint(obj.v));
}

void frag_main() {
  S v_2 = {1.0f, 2u, (3.0f).xxxx};
  v_1(0u, v_2);
}

