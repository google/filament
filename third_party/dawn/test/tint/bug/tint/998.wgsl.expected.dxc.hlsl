cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};

struct S {
  uint data[3];
};

static S s = (S)0;

[numthreads(1, 1, 1)]
void main() {
  {
    uint tint_symbol_1[3] = s.data;
    tint_symbol_1[min(constants[0].x, 2u)] = 0u;
    s.data = tint_symbol_1;
  }
  return;
}
