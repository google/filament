
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
  uint v = uniforms[0u].x;
  switch(v) {
    case 0u:
    {
      m1[0u].x = 1.0f;
      break;
    }
    case 1u:
    {
      m1[1u].x = 1.0f;
      break;
    }
    default:
    {
      break;
    }
  }
}

