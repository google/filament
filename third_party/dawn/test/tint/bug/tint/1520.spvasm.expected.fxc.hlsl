int tint_ftoi(float v) {
  return ((v <= 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

cbuffer cbuffer_x_4 : register(b0) {
  uint4 x_4[7];
};
static float4 sk_FragColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
static bool sk_Clockwise = false;
static float4 vcolor_S0 = float4(0.0f, 0.0f, 0.0f, 0.0f);

int4 tint_div(int4 lhs, int4 rhs) {
  return (lhs / (((rhs == (0).xxxx) | ((lhs == (-2147483648).xxxx) & (rhs == (-1).xxxx))) ? (1).xxxx : rhs));
}

bool test_int_S1_c0_b() {
  int unknown = 0;
  bool ok = false;
  int4 val = int4(0, 0, 0, 0);
  bool x_40 = false;
  bool x_41 = false;
  bool x_54 = false;
  bool x_55 = false;
  bool x_65 = false;
  bool x_66 = false;
  int x_27 = tint_ftoi(asfloat(x_4[1].x));
  unknown = x_27;
  ok = true;
  x_41 = false;
  if (true) {
    x_40 = all((tint_div((0).xxxx, int4((x_27).xxxx)) == (0).xxxx));
    x_41 = x_40;
  }
  ok = x_41;
  int4 x_44 = int4((x_27).xxxx);
  val = x_44;
  int4 x_47 = (x_44 + (1).xxxx);
  val = x_47;
  int4 x_48 = (x_47 - (1).xxxx);
  val = x_48;
  int4 x_49 = (x_48 + (1).xxxx);
  val = x_49;
  int4 x_50 = (x_49 - (1).xxxx);
  val = x_50;
  x_55 = false;
  if (x_41) {
    x_54 = all((x_50 == x_44));
    x_55 = x_54;
  }
  ok = x_55;
  int4 x_58 = (x_50 * (2).xxxx);
  val = x_58;
  int4 x_59 = tint_div(x_58, (2).xxxx);
  val = x_59;
  int4 x_60 = (x_59 * (2).xxxx);
  val = x_60;
  int4 x_61 = tint_div(x_60, (2).xxxx);
  val = x_61;
  x_66 = false;
  if (x_55) {
    x_65 = all((x_61 == x_44));
    x_66 = x_65;
  }
  ok = x_66;
  return x_66;
}

void main_1() {
  float4 outputColor_S0 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 output_S1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float x_8_unknown = 0.0f;
  bool x_9_ok = false;
  float4 x_10_val = float4(0.0f, 0.0f, 0.0f, 0.0f);
  float4 x_116 = float4(0.0f, 0.0f, 0.0f, 0.0f);
  bool x_86 = false;
  bool x_87 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_110 = false;
  bool x_111 = false;
  bool x_114 = false;
  bool x_115 = false;
  outputColor_S0 = vcolor_S0;
  float x_77 = asfloat(x_4[1].x);
  x_8_unknown = x_77;
  x_9_ok = true;
  x_87 = false;
  if (true) {
    x_86 = all((((0.0f).xxxx / float4((x_77).xxxx)) == (0.0f).xxxx));
    x_87 = x_86;
  }
  x_9_ok = x_87;
  float4 x_89 = float4((x_77).xxxx);
  x_10_val = x_89;
  float4 x_92 = (x_89 + (1.0f).xxxx);
  x_10_val = x_92;
  float4 x_93 = (x_92 - (1.0f).xxxx);
  x_10_val = x_93;
  float4 x_94 = (x_93 + (1.0f).xxxx);
  x_10_val = x_94;
  float4 x_95 = (x_94 - (1.0f).xxxx);
  x_10_val = x_95;
  x_100 = false;
  if (x_87) {
    x_99 = all((x_95 == x_89));
    x_100 = x_99;
  }
  x_9_ok = x_100;
  float4 x_103 = (x_95 * (2.0f).xxxx);
  x_10_val = x_103;
  float4 x_104 = (x_103 / (2.0f).xxxx);
  x_10_val = x_104;
  float4 x_105 = (x_104 * (2.0f).xxxx);
  x_10_val = x_105;
  float4 x_106 = (x_105 / (2.0f).xxxx);
  x_10_val = x_106;
  x_111 = false;
  if (x_100) {
    x_110 = all((x_106 == x_89));
    x_111 = x_110;
  }
  x_9_ok = x_111;
  x_115 = false;
  if (x_111) {
    x_114 = test_int_S1_c0_b();
    x_115 = x_114;
  }
  if (x_115) {
    x_116 = asfloat(x_4[3]);
  } else {
    x_116 = asfloat(x_4[2]);
  }
  float4 x_125 = x_116;
  output_S1 = x_116;
  sk_FragColor = x_125;
  return;
}

struct main_out {
  float4 sk_FragColor_1;
};
struct tint_symbol_1 {
  float4 vcolor_S0_param : TEXCOORD0;
  bool sk_Clockwise_param : SV_IsFrontFace;
};
struct tint_symbol_2 {
  float4 sk_FragColor_1 : SV_Target0;
};

main_out main_inner(bool sk_Clockwise_param, float4 vcolor_S0_param) {
  sk_Clockwise = sk_Clockwise_param;
  vcolor_S0 = vcolor_S0_param;
  main_1();
  main_out tint_symbol_3 = {sk_FragColor};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  main_out inner_result = main_inner(tint_symbol.sk_Clockwise_param, tint_symbol.vcolor_S0_param);
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.sk_FragColor_1 = inner_result.sk_FragColor_1;
  return wrapper_result;
}
