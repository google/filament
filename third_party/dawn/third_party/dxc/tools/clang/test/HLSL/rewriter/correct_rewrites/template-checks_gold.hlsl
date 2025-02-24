// Rewrite unchanged result:
Texture2D<float4> t_float4;
matrix<bool, 1, 2> m_bool;
struct s_float2_float3 {
  float2 f_float2;
  float3 f_float3;
};
struct s_float4_sampler {
  float4 f_float4;
  SamplerState f_sampler;
};
void vain() {
  AppendStructuredBuffer<float4> asb;
  float4 f4;
  asb.Append(f4);
}


