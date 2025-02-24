struct str {
  float4 i;
};

void func(inout float4 pointer) {
  pointer = (0.0f).xxxx;
}

static str P = (str)0;

[numthreads(1, 1, 1)]
void main() {
  func(P.i);
  return;
}
