float2 func(inout float2 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  float2x2 F = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  float2 r = func(F[1]);
  return;
}
