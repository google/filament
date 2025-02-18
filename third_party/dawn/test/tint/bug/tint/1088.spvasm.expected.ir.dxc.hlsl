struct main_out {
  float4 gl_Position;
  float2 vUV_1;
};

struct main_outputs {
  float2 main_out_vUV_1 : TEXCOORD0;
  float4 main_out_gl_Position : SV_Position;
};

struct main_inputs {
  float3 position_1_param : TEXCOORD0;
  float3 normal_param : TEXCOORD1;
  float2 uv_param : TEXCOORD2;
};


static float3 position_1 = (0.0f).xxx;
cbuffer cbuffer_x_14 : register(b2, space2) {
  uint4 x_14[17];
};
static float2 vUV = (0.0f).xx;
static float2 uv = (0.0f).xx;
static float3 normal = (0.0f).xxx;
static float4 gl_Position = (0.0f).xxxx;
float4x4 v(uint start_byte_offset) {
  return float4x4(asfloat(x_14[(start_byte_offset / 16u)]), asfloat(x_14[((16u + start_byte_offset) / 16u)]), asfloat(x_14[((32u + start_byte_offset) / 16u)]), asfloat(x_14[((48u + start_byte_offset) / 16u)]));
}

void main_1() {
  float4 q = (0.0f).xxxx;
  float3 p = (0.0f).xxx;
  q = float4(position_1.x, position_1.y, position_1.z, 1.0f);
  p = q.xyz;
  p.x = (p.x + sin(((asfloat(x_14[13u].x) * position_1.y) + asfloat(x_14[4u].x))));
  p.y = (p.y + sin((asfloat(x_14[4u].x) + 4.0f)));
  float4x4 v_1 = v(0u);
  gl_Position = mul(float4(p.x, p.y, p.z, 1.0f), v_1);
  vUV = uv;
  gl_Position.y = (gl_Position.y * -1.0f);
}

main_out main_inner(float3 position_1_param, float2 uv_param, float3 normal_param) {
  position_1 = position_1_param;
  uv = uv_param;
  normal = normal_param;
  main_1();
  main_out v_2 = {gl_Position, vUV};
  return v_2;
}

main_outputs main(main_inputs inputs) {
  main_out v_3 = main_inner(inputs.position_1_param, inputs.uv_param, inputs.normal_param);
  main_outputs v_4 = {v_3.vUV_1, v_3.gl_Position};
  return v_4;
}

