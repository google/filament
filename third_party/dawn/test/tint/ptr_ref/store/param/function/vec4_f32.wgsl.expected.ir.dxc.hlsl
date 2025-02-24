
void func(inout float4 pointer) {
  pointer = (0.0f).xxxx;
}

[numthreads(1, 1, 1)]
void main() {
  float4 F = (0.0f).xxxx;
  func(F);
}

