static float3 position_1 = float3(0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_14 : register(b2, space2) {
  uint4 x_14[17];
};
static float2 vUV = float2(0.0f, 0.0f);
static float2 uv = float2(0.0f, 0.0f);
static float3 normal = float3(0.0f, 0.0f, 0.0f);
static float4 gl_Position = float4(0.0f, 0.0f, 0.0f, 0.0f);

float4x4 x_14_load_1(uint offset) {
  const uint scalar_offset = ((offset + 0u)) / 4;
  const uint scalar_offset_1 = ((offset + 16u)) / 4;
  const uint scalar_offset_2 = ((offset + 32u)) / 4;
  const uint scalar_offset_3 = ((offset + 48u)) / 4;
  return float4x4(asfloat(x_14[scalar_offset / 4]), asfloat(x_14[scalar_offset_1 / 4]), asfloat(x_14[scalar_offset_2 / 4]), asfloat(x_14[scalar_offset_3 / 4]));
}

void main_1() {
  float4 q = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float3 p = float3(0.0f, 0.0f, 0.0f);
  q = float4(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  p.x = (p.x + sin(((asfloat(x_14[13].x) * position_1.y) + asfloat(x_14[4].x))));
  p.y = (p.y + sin((asfloat(x_14[4].x) + 4.0f)));
  gl_Position = mul(float4(p.x, p.y, p.z, 1.0f), x_14_load_1(0u));
  vUV = uv;
  gl_Position.y = (gl_Position.y * -1.0f);
  return;
}

struct main_out {
  float4 gl_Position;
  float2 vUV_1;
};
struct tint_symbol_1 {
  float3 position_1_param : TEXCOORD0;
  float3 normal_param : TEXCOORD1;
  float2 uv_param : TEXCOORD2;
};
struct tint_symbol_2 {
  float2 vUV_1 : TEXCOORD0;
  float4 gl_Position : SV_Position;
};

main_out main_inner(float3 position_1_param, float2 uv_param, float3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  main_out tint_symbol_3 = {gl_Position, vUV};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  main_out inner_result = main_inner(tint_symbol.position_1_param, tint_symbol.uv_param, tint_symbol.normal_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.gl_Position = inner_result.gl_Position;
  wrapper_result.vUV_1 = inner_result.vUV_1;
  return wrapper_result;
}
