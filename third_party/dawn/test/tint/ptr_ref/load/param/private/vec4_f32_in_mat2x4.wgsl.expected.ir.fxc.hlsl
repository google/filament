
static float2x4 P = float2x4((0.0f).xxxx, (0.0f).xxxx);
float4 func(inout float4 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  float4 r = func(P[1u]);
}

