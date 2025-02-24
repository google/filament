struct S {
  float4 a;
  int b;
};


ByteAddressBuffer sb : register(t0);
S v(uint offset) {
  S v_1 = {asfloat(sb.Load4((offset + 0u))), asint(sb.Load((offset + 16u)))};
  return v_1;
}

[numthreads(1, 1, 1)]
void main() {
  uint v_2 = 0u;
  sb.GetDimensions(v_2);
  uint v_3 = ((v_2 / 32u) - 1u);
  S x = v((0u + (min(uint(int(1)), v_3) * 32u)));
}

