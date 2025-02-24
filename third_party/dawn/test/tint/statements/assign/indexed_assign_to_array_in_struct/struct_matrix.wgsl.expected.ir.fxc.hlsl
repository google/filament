struct OuterS {
  float2x4 m1;
};


cbuffer cbuffer_uniforms : register(b4, space1) {
  uint4 uniforms[1];
};
[numthreads(1, 1, 1)]
void main() {
  OuterS s1 = (OuterS)0;
  uint v = uniforms[0u].x;
  switch(v) {
    case 0u:
    {
      s1.m1[0u] = (1.0f).xxxx;
      break;
    }
    case 1u:
    {
      s1.m1[1u] = (1.0f).xxxx;
      break;
    }
    default:
    {
      break;
    }
  }
  uint v_1 = uniforms[0u].x;
  uint v_2 = uniforms[0u].x;
  switch(v_1) {
    case 0u:
    {
      float4 v_3 = s1.m1[0u];
      float4 v_4 = float4((1.0f).xxxx);
      float4 v_5 = float4((v_2).xxxx);
      s1.m1[0u] = (((v_5 == float4(int(0), int(1), int(2), int(3)))) ? (v_4) : (v_3));
      break;
    }
    case 1u:
    {
      float4 v_6 = s1.m1[1u];
      float4 v_7 = float4((1.0f).xxxx);
      float4 v_8 = float4((v_2).xxxx);
      s1.m1[1u] = (((v_8 == float4(int(0), int(1), int(2), int(3)))) ? (v_7) : (v_6));
      break;
    }
    default:
    {
      break;
    }
  }
}

