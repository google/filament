struct main_out {
  float4 sk_FragColor_1;
};

struct main_outputs {
  float4 main_out_sk_FragColor_1 : SV_Target0;
};

struct main_inputs {
  float4 vcolor_S0_param : TEXCOORD0;
  bool sk_Clockwise_param : SV_IsFrontFace;
};


cbuffer cbuffer_x_4 : register(b0) {
  uint4 x_4[7];
};
static float4 sk_FragColor = (0.0f).xxxx;
static bool sk_Clockwise = false;
static float4 vcolor_S0 = (0.0f).xxxx;
int4 tint_div_v4i32(int4 lhs, int4 rhs) {
  return (lhs / ((((rhs == (int(0)).xxxx) | ((lhs == (int(-2147483648)).xxxx) & (rhs == (int(-1)).xxxx)))) ? ((int(1)).xxxx) : (rhs)));
}

int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

bool test_int_S1_c0_b() {
  int unknown = int(0);
  bool ok = false;
  int4 val = (int(0)).xxxx;
  bool x_40 = false;
  bool x_41 = false;
  bool x_54 = false;
  bool x_55 = false;
  bool x_65 = false;
  bool x_66 = false;
  int x_27 = tint_f32_to_i32(asfloat(x_4[1u].x));
  unknown = x_27;
  ok = true;
  x_41 = false;
  if (true) {
    x_40 = all((tint_div_v4i32((int(0)).xxxx, int4((x_27).xxxx)) == (int(0)).xxxx));
    x_41 = x_40;
  }
  ok = x_41;
  int4 x_44 = int4((x_27).xxxx);
  val = x_44;
  int4 x_47 = (x_44 + (int(1)).xxxx);
  val = x_47;
  int4 x_48 = (x_47 - (int(1)).xxxx);
  val = x_48;
  int4 x_49 = (x_48 + (int(1)).xxxx);
  val = x_49;
  int4 x_50 = (x_49 - (int(1)).xxxx);
  val = x_50;
  x_55 = false;
  if (x_41) {
    x_54 = all((x_50 == x_44));
    x_55 = x_54;
  }
  ok = x_55;
  int4 x_58 = (x_50 * (int(2)).xxxx);
  val = x_58;
  int4 x_59 = tint_div_v4i32(x_58, (int(2)).xxxx);
  val = x_59;
  int4 x_60 = (x_59 * (int(2)).xxxx);
  val = x_60;
  int4 x_61 = tint_div_v4i32(x_60, (int(2)).xxxx);
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
  float4 outputColor_S0 = (0.0f).xxxx;
  float4 output_S1 = (0.0f).xxxx;
  float x_8_unknown = 0.0f;
  bool x_9_ok = false;
  float4 x_10_val = (0.0f).xxxx;
  float4 x_116 = (0.0f).xxxx;
  bool x_86 = false;
  bool x_87 = false;
  bool x_99 = false;
  bool x_100 = false;
  bool x_110 = false;
  bool x_111 = false;
  bool x_114 = false;
  bool x_115 = false;
  outputColor_S0 = vcolor_S0;
  float x_77 = asfloat(x_4[1u].x);
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
    x_116 = asfloat(x_4[3u]);
  } else {
    x_116 = asfloat(x_4[2u]);
  }
  float4 x_125 = x_116;
  output_S1 = x_116;
  sk_FragColor = x_125;
}

main_out main_inner(bool sk_Clockwise_param, float4 vcolor_S0_param) {
  sk_Clockwise = sk_Clockwise_param;
  vcolor_S0 = vcolor_S0_param;
  main_1();
  main_out v = {sk_FragColor};
  return v;
}

main_outputs main(main_inputs inputs) {
  main_out v_1 = main_inner(inputs.sk_Clockwise_param, inputs.vcolor_S0_param);
  main_outputs v_2 = {v_1.sk_FragColor_1};
  return v_2;
}

