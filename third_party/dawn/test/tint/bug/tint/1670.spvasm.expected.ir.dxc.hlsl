
void main_1() {
  float2x2 m2i = float2x2((0.0f).xx, (0.0f).xx);
  float2x2 m2 = float2x2((0.0f).xx, (0.0f).xx);
  float3x3 m3i = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float3x3 m3 = float3x3((0.0f).xxx, (0.0f).xxx, (0.0f).xxx);
  float4x4 m4i = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float4x4 m4 = float4x4((0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx, (0.0f).xxxx);
  float s = (1.0f / determinant(m2));
  float2 v = float2((s * m2[1u].y), (-(s) * m2[0u].y));
  m2i = float2x2(v, float2((-(s) * m2[1u].x), (s * m2[0u].x)));
  float v_1 = (1.0f / determinant(m3));
  float3 v_2 = float3(((m3[1u].y * m3[2u].z) - (m3[1u].z * m3[2u].y)), ((m3[0u].z * m3[2u].y) - (m3[0u].y * m3[2u].z)), ((m3[0u].y * m3[1u].z) - (m3[0u].z * m3[1u].y)));
  float3 v_3 = float3(((m3[1u].z * m3[2u].x) - (m3[1u].x * m3[2u].z)), ((m3[0u].x * m3[2u].z) - (m3[0u].z * m3[2u].x)), ((m3[0u].z * m3[1u].x) - (m3[0u].x * m3[1u].z)));
  m3i = (v_1 * float3x3(v_2, v_3, float3(((m3[1u].x * m3[2u].y) - (m3[1u].y * m3[2u].x)), ((m3[0u].y * m3[2u].x) - (m3[0u].x * m3[2u].y)), ((m3[0u].x * m3[1u].y) - (m3[0u].y * m3[1u].x)))));
  float v_4 = (1.0f / determinant(m4));
  float4 v_5 = float4((((m4[1u].y * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) - (m4[1u].z * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y)))) + (m4[1u].w * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y)))), (((-(m4[0u].y) * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) + (m4[0u].z * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y)))) - (m4[0u].w * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y)))), (((m4[0u].y * ((m4[1u].z * m4[3u].w) - (m4[1u].w * m4[3u].z))) - (m4[0u].z * ((m4[1u].y * m4[3u].w) - (m4[1u].w * m4[3u].y)))) + (m4[0u].w * ((m4[1u].y * m4[3u].z) - (m4[1u].z * m4[3u].y)))), (((-(m4[0u].y) * ((m4[1u].z * m4[2u].w) - (m4[1u].w * m4[2u].z))) + (m4[0u].z * ((m4[1u].y * m4[2u].w) - (m4[1u].w * m4[2u].y)))) - (m4[0u].w * ((m4[1u].y * m4[2u].z) - (m4[1u].z * m4[2u].y)))));
  float4 v_6 = float4((((-(m4[1u].x) * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) + (m4[1u].z * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) - (m4[1u].w * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))), (((m4[0u].x * ((m4[2u].z * m4[3u].w) - (m4[2u].w * m4[3u].z))) - (m4[0u].z * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) + (m4[0u].w * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))), (((-(m4[0u].x) * ((m4[1u].z * m4[3u].w) - (m4[1u].w * m4[3u].z))) + (m4[0u].z * ((m4[1u].x * m4[3u].w) - (m4[1u].w * m4[3u].x)))) - (m4[0u].w * ((m4[1u].x * m4[3u].z) - (m4[1u].z * m4[3u].x)))), (((m4[0u].x * ((m4[1u].z * m4[2u].w) - (m4[1u].w * m4[2u].z))) - (m4[0u].z * ((m4[1u].x * m4[2u].w) - (m4[1u].w * m4[2u].x)))) + (m4[0u].w * ((m4[1u].x * m4[2u].z) - (m4[1u].z * m4[2u].x)))));
  float4 v_7 = float4((((m4[1u].x * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y))) - (m4[1u].y * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) + (m4[1u].w * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((-(m4[0u].x) * ((m4[2u].y * m4[3u].w) - (m4[2u].w * m4[3u].y))) + (m4[0u].y * ((m4[2u].x * m4[3u].w) - (m4[2u].w * m4[3u].x)))) - (m4[0u].w * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((m4[0u].x * ((m4[1u].y * m4[3u].w) - (m4[1u].w * m4[3u].y))) - (m4[0u].y * ((m4[1u].x * m4[3u].w) - (m4[1u].w * m4[3u].x)))) + (m4[0u].w * ((m4[1u].x * m4[3u].y) - (m4[1u].y * m4[3u].x)))), (((-(m4[0u].x) * ((m4[1u].y * m4[2u].w) - (m4[1u].w * m4[2u].y))) + (m4[0u].y * ((m4[1u].x * m4[2u].w) - (m4[1u].w * m4[2u].x)))) - (m4[0u].w * ((m4[1u].x * m4[2u].y) - (m4[1u].y * m4[2u].x)))));
  m4i = (v_4 * float4x4(v_5, v_6, v_7, float4((((-(m4[1u].x) * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y))) + (m4[1u].y * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))) - (m4[1u].z * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((m4[0u].x * ((m4[2u].y * m4[3u].z) - (m4[2u].z * m4[3u].y))) - (m4[0u].y * ((m4[2u].x * m4[3u].z) - (m4[2u].z * m4[3u].x)))) + (m4[0u].z * ((m4[2u].x * m4[3u].y) - (m4[2u].y * m4[3u].x)))), (((-(m4[0u].x) * ((m4[1u].y * m4[3u].z) - (m4[1u].z * m4[3u].y))) + (m4[0u].y * ((m4[1u].x * m4[3u].z) - (m4[1u].z * m4[3u].x)))) - (m4[0u].z * ((m4[1u].x * m4[3u].y) - (m4[1u].y * m4[3u].x)))), (((m4[0u].x * ((m4[1u].y * m4[2u].z) - (m4[1u].z * m4[2u].y))) - (m4[0u].y * ((m4[1u].x * m4[2u].z) - (m4[1u].z * m4[2u].x)))) + (m4[0u].z * ((m4[1u].x * m4[2u].y) - (m4[1u].y * m4[2u].x)))))));
}

void main() {
  main_1();
}

