
cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
static float2x4 m1 = float2x4((0.0f).xxxx, (0.0f).xxxx);
[numthreads(1, 1, 1)]
void main() {
  uint v = uniforms[0u].x;
  uint v_1 = uniforms[0u].y;
  switch(v) {
    case 0u:
    {
      float4 v_2 = m1[0u];
      float4 v_3 = float4((1.0f).xxxx);
      float4 v_4 = float4((v_1).xxxx);
      m1[0u] = (((v_4 == float4(int(0), int(1), int(2), int(3)))) ? (v_3) : (v_2));
      break;
    }
    case 1u:
    {
      float4 v_5 = m1[1u];
      float4 v_6 = float4((1.0f).xxxx);
      float4 v_7 = float4((v_1).xxxx);
      m1[1u] = (((v_7 == float4(int(0), int(1), int(2), int(3)))) ? (v_6) : (v_5));
      break;
    }
    default:
    {
      break;
    }
  }
}

