float2 func(inout float2 pointer) {
  return pointer;
}

static float2x2 P = float2x2(0.0f, 0.0f, 0.0f, 0.0f);

[numthreads(1, 1, 1)]
void main() {
  float2 r = func(P[1]);
  return;
}
