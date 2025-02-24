struct S {
  uint data[3];
};


cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};
static S s = (S)0;
[numthreads(1, 1, 1)]
void main() {
  uint v = constants[0u].x;
  uint tint_array_copy[3] = s.data;
  tint_array_copy[min(v, 2u)] = 0u;
  uint v_1[3] = tint_array_copy;
  s.data = v_1;
}

