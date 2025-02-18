float4 func(inout float4 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  float4 F = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 r = func(F);
  return;
}
