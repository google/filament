struct S {
  uint data[3];
};


cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};
static S s = (S)0;
[numthreads(1, 1, 1)]
void main() {
  uint v = min(constants[0u].x, 2u);
  s.data[v] = 0u;
}

