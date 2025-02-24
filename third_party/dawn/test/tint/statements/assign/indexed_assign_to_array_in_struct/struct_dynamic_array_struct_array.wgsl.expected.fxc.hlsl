struct InnerS {
  int v;
};

cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s : register(u0);

void s_store(uint offset, InnerS value) {
  s.Store((offset + 0u), asuint(value.v));
}

[numthreads(1, 1, 1)]
void main() {
  uint tint_symbol_1 = 0u;
  s.GetDimensions(tint_symbol_1);
  uint tint_symbol_2 = ((tint_symbol_1 - 0u) / 32u);
  InnerS v = (InnerS)0;
  s_store(((32u * min(uniforms[0].x, (tint_symbol_2 - 1u))) + (4u * min(uniforms[0].y, 7u))), v);
  return;
}
