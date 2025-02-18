
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
  uint v = min(uniforms[0u].x, 1u);
  m1[v].x = 1.0f;
}

