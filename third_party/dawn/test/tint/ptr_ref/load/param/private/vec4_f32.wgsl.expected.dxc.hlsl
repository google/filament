float4 func(inout float4 pointer) {
  return pointer;
}

static float4 P = float4(0.0f, 0.0f, 0.0f, 0.0f);

[numthreads(1, 1, 1)]
void main() {
  float4 r = func(P);
  return;
}
