struct S {
  float f;
};


ByteAddressBuffer v : register(t0);
RWByteAddressBuffer v_1 : register(u1);
void v_2(uint offset, S obj) {
  v_1.Store((offset + 0u), asuint(obj.f));
}

S v_3(uint offset) {
  S v_4 = {asfloat(v.Load((offset + 0u)))};
  return v_4;
}

[numthreads(1, 1, 1)]
void main() {
  S v_5 = v_3(0u);
  v_2(0u, v_5);
}

