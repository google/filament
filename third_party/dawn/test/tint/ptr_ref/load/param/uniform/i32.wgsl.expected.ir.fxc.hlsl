
cbuffer cbuffer_S : register(b0) {
  uint4 S[1];
};
int func() {
  return asint(S[0u].x);
}

[numthreads(1, 1, 1)]
void main() {
  int r = func();
}

