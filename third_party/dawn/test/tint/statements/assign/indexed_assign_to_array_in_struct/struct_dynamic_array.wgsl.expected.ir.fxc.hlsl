struct InnerS {
  int v;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
RWByteAddressBuffer s1 : register(u0);
void v_1(uint offset, InnerS obj) {
  s1.Store((offset + 0u), asuint(obj.v));
}

[numthreads(1, 1, 1)]
void main() {
  InnerS v = (InnerS)0;
  uint v_2 = 0u;
  s1.GetDimensions(v_2);
  InnerS v_3 = v;
  v_1((0u + (min(uniforms[0u].x, ((v_2 / 4u) - 1u)) * 4u)), v_3);
}

