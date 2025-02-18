struct InnerS {
  int v;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s : register(u0);
void v_1(uint offset, InnerS obj) {
  s.Store((offset + 0u), asuint(obj.v));
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  uint v_2 = 0u;
  s.GetDimensions(v_2);
  InnerS v_3 = v;
  v_1(((0u + (min(uniforms[0u].x, ((v_2 / 32u) - 1u)) * 32u)) + (min(uniforms[0u].y, 7u) * 4u)), v_3);
}

