struct Mat4x4_ {
  float4 mx;
  float4 my;
  float4 mz;
  float4 mw;
};

struct Mat4x2_ {
  float4 mx;
  float4 my;
};

struct Mat4x3_ {
  float4 mx;
  float4 my;
  float4 mz;
};

struct VertexOutput {
  float4 v_Color;
  float2 v_TexCoord;
  float4 member;
};

struct main_outputs {
  float4 VertexOutput_v_Color : TEXCOORD0;
  float2 VertexOutput_v_TexCoord : TEXCOORD1;
  float4 VertexOutput_member : SV_Position;
};

struct main_inputs {
  float3 a_Position : TEXCOORD0;
  float2 a_UV : TEXCOORD1;
  float4 a_Color : TEXCOORD2;
  float3 a_Normal : TEXCOORD3;
  float a_PosMtxIdx : TEXCOORD4;
};


cbuffer cbuffer_global : register(b0) {
  uint4 global[4];
};
cbuffer cbuffer_global1 : register(b1) {
  uint4 global1[3];
};
cbuffer cbuffer_global2 : register(b2) {
  uint4 global2[96];
};
static float3 a_Position1 = (0.0f).xxx;
static float2 a_UV1 = (0.0f).xx;
static float4 a_Color1 = (0.0f).xxxx;
static float3 a_Normal1 = (0.0f).xxx;
static float a_PosMtxIdx1 = 0.0f;
static float4 v_Color = (0.0f).xxxx;
static float2 v_TexCoord = (0.0f).xx;
static float4 gl_Position = (0.0f).xxxx;
float4 Mul(Mat4x4_ m8, float4 v) {
  Mat4x4_ m9 = (Mat4x4_)0;
  float4 v1 = (0.0f).xxxx;
  m9 = m8;
  v1 = v;
  Mat4x4_ x_e4 = m9;
  float4 x_e6 = v1;
  Mat4x4_ x_e8 = m9;
  float4 x_e10 = v1;
  Mat4x4_ x_e12 = m9;
  float4 x_e14 = v1;
  Mat4x4_ x_e16 = m9;
  float4 x_e18 = v1;
  return float4(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10), dot(x_e12.mz, x_e14), dot(x_e16.mw, x_e18));
}

float2 Mul2(Mat4x2_ m12, float4 v4) {
  Mat4x2_ m13 = (Mat4x2_)0;
  float4 v5 = (0.0f).xxxx;
  m13 = m12;
  v5 = v4;
  Mat4x2_ x_e4 = m13;
  float4 x_e6 = v5;
  Mat4x2_ x_e8 = m13;
  float4 x_e10 = v5;
  return float2(dot(x_e4.mx, x_e6), dot(x_e8.my, x_e10));
}

Mat4x4_ x_Mat4x4_(float n) {
  float n1 = 0.0f;
  Mat4x4_ o = (Mat4x4_)0;
  n1 = n;
  float x_e4 = n1;
  o.mx = float4(x_e4, 0.0f, 0.0f, 0.0f);
  float x_e11 = n1;
  o.my = float4(0.0f, x_e11, 0.0f, 0.0f);
  float x_e18 = n1;
  o.mz = float4(0.0f, 0.0f, x_e18, 0.0f);
  float x_e25 = n1;
  o.mw = float4(0.0f, 0.0f, 0.0f, x_e25);
  Mat4x4_ x_e27 = o;
  return x_e27;
}

Mat4x4_ x_Mat4x4_1(Mat4x3_ m16) {
  Mat4x3_ m17 = (Mat4x3_)0;
  Mat4x4_ o1 = (Mat4x4_)0;
  m17 = m16;
  Mat4x4_ x_e4 = x_Mat4x4_(1.0f);
  o1 = x_e4;
  Mat4x3_ x_e7 = m17;
  o1.mx = x_e7.mx;
  Mat4x3_ x_e10 = m17;
  o1.my = x_e10.my;
  Mat4x3_ x_e13 = m17;
  o1.mz = x_e13.mz;
  Mat4x4_ x_e15 = o1;
  return x_e15;
}

Mat4x2_ v_1(uint start_byte_offset) {
  Mat4x2_ v_2 = {asfloat(global1[(start_byte_offset / 16u)]), asfloat(global1[((16u + start_byte_offset) / 16u)])};
  return v_2;
}

Mat4x4_ v_3(uint start_byte_offset) {
  Mat4x4_ v_4 = {asfloat(global[(start_byte_offset / 16u)]), asfloat(global[((16u + start_byte_offset) / 16u)]), asfloat(global[((32u + start_byte_offset) / 16u)]), asfloat(global[((48u + start_byte_offset) / 16u)])};
  return v_4;
}

Mat4x3_ v_5(uint start_byte_offset) {
  Mat4x3_ v_6 = {asfloat(global2[(start_byte_offset / 16u)]), asfloat(global2[((16u + start_byte_offset) / 16u)]), asfloat(global2[((32u + start_byte_offset) / 16u)])};
  return v_6;
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void main1() {
  Mat4x3_ t_PosMtx = (Mat4x3_)0;
  float2 t_TexSpaceCoord = (0.0f).xx;
  float x_e15 = a_PosMtxIdx1;
  Mat4x3_ x_e18 = v_5((48u * min(uint(tint_f32_to_i32(x_e15)), 31u)));
  t_PosMtx = x_e18;
  Mat4x3_ x_e23 = t_PosMtx;
  Mat4x4_ x_e24 = x_Mat4x4_1(x_e23);
  float3 x_e25 = a_Position1;
  Mat4x3_ x_e29 = t_PosMtx;
  Mat4x4_ x_e30 = x_Mat4x4_1(x_e29);
  float3 x_e31 = a_Position1;
  float4 x_e34 = Mul(x_e30, float4(x_e31, 1.0f));
  Mat4x4_ x_e35 = v_3(0u);
  Mat4x3_ x_e37 = t_PosMtx;
  Mat4x4_ x_e38 = x_Mat4x4_1(x_e37);
  float3 x_e39 = a_Position1;
  Mat4x3_ x_e43 = t_PosMtx;
  Mat4x4_ x_e44 = x_Mat4x4_1(x_e43);
  float3 x_e45 = a_Position1;
  float4 x_e48 = Mul(x_e44, float4(x_e45, 1.0f));
  float4 x_e49 = Mul(x_e35, x_e48);
  gl_Position = x_e49;
  float4 x_e50 = a_Color1;
  v_Color = x_e50;
  float4 x_e52 = asfloat(global1[2u]);
  if ((x_e52.x == 2.0f)) {
    float3 x_e59 = a_Normal1;
    Mat4x2_ x_e64 = v_1(0u);
    float3 x_e65 = a_Normal1;
    float2 x_e68 = Mul2(x_e64, float4(x_e65, 1.0f));
    v_TexCoord = x_e68.xy;
    return;
  } else {
    float2 x_e73 = a_UV1;
    Mat4x2_ x_e79 = v_1(0u);
    float2 x_e80 = a_UV1;
    float2 x_e84 = Mul2(x_e79, float4(x_e80, 1.0f, 1.0f));
    v_TexCoord = x_e84.xy;
    return;
  }
  /* unreachable */
}

VertexOutput main_inner(float3 a_Position, float2 a_UV, float4 a_Color, float3 a_Normal, float a_PosMtxIdx) {
  a_Position1 = a_Position;
  a_UV1 = a_UV;
  a_Color1 = a_Color;
  a_Normal1 = a_Normal;
  a_PosMtxIdx1 = a_PosMtxIdx;
  main1();
  float4 x_e11 = v_Color;
  float2 x_e13 = v_TexCoord;
  float4 x_e15 = gl_Position;
  VertexOutput v_7 = {x_e11, x_e13, x_e15};
  return v_7;
}

main_outputs main(main_inputs inputs) {
  VertexOutput v_8 = main_inner(inputs.a_Position, inputs.a_UV, inputs.a_Color, inputs.a_Normal, inputs.a_PosMtxIdx);
  main_outputs v_9 = {v_8.v_Color, v_8.v_TexCoord, v_8.member};
  return v_9;
}

