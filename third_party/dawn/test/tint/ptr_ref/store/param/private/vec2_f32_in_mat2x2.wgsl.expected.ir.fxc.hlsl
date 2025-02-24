
static float2x2 P = float2x2((0.0f).xx, (0.0f).xx);
void func(inout float2 pointer) {
  pointer = (0.0f).xx;
}

[numthreads(1, 1, 1)]
void main() {
  func(P[1u]);
}

