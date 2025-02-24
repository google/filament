
float4 func(inout float4 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  float4 F = (0.0f).xxxx;
  float4 r = func(F);
}

