void func(inout float2 pointer) {
  pointer = (0.0f).xx;
}

[numthreads(1, 1, 1)]
void main() {
  float2x2 F = float2x2(0.0f, 0.0f, 0.0f, 0.0f);
  func(F[1]);
  return;
}
