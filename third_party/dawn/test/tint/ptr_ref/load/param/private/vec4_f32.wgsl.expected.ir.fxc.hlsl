
static float4 P = (0.0f).xxxx;
float4 func(inout float4 pointer) {
  return pointer;
}

[numthreads(1, 1, 1)]
void main() {
  float4 r = func(P);
}

