
static float4 P = (0.0f).xxxx;
void func(inout float4 pointer) {
  pointer = (0.0f).xxxx;
}

[numthreads(1, 1, 1)]
void main() {
  func(P);
}

