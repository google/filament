struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
void main_1() {
  float f = 0.0f;
  float4 v = (0.0f).xxxx;
  f = 1.0f;
  v = float4(sin(f), cos(f), exp2(f), log(f));
  if ((distance(v, asfloat(x_7[0u])) < 0.10000000149011611938f)) {
    x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  } else {
    x_GLF_color = (0.0f).xxxx;
  }
}

main_out main_inner() {
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main() {
  main_out v_2 = main_inner();
  main_outputs v_3 = {v_2.x_GLF_color_1};
  return v_3;
}

