void func(inout float2 pointer) {
  pointer = (0.0f).xx;
}

static float2x2 P = float2x2(0.0f, 0.0f, 0.0f, 0.0f);

[numthreads(1, 1, 1)]
void main() {
  func(P[1]);
  return;
}
