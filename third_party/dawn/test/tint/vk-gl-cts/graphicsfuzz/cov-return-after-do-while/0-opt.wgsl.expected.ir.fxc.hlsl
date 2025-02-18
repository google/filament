SKIP: INVALID

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static float4 x_GLF_color = (0.0f).xxxx;
cbuffer cbuffer_x_5 : register(b0) {
  uint4 x_5[2];
};
void main_1() {
  int x_22 = asint(x_5[0u].x);
  int x_25 = asint(x_5[1u].x);
  int x_28 = asint(x_5[1u].x);
  int x_31 = asint(x_5[0u].x);
  float v = float(x_22);
  float v_1 = float(x_25);
  float v_2 = float(x_28);
  x_GLF_color = float4(v, v_1, v_2, float(x_31));
  int x_35 = asint(x_5[1u].x);
  int x_37 = asint(x_5[0u].x);
  if ((x_35 > x_37)) {
    {
      while(true) {
        int x_46 = asint(x_5[0u].x);
        float x_47 = float(x_46);
        x_GLF_color = float4(x_47, x_47, x_47, x_47);
        {
          int x_50 = asint(x_5[1u].x);
          int x_52 = asint(x_5[0u].x);
          if (!((x_50 > x_52))) { break; }
        }
        continue;
      }
    }
    return;
  }
}

main_out main_inner() {
  main_1();
  main_out v_3 = {x_GLF_color};
  return v_3;
}

main_outputs main() {
  main_out v_4 = main_inner();
  main_outputs v_5 = {v_4.x_GLF_color_1};
  return v_5;
}

FXC validation failure:
<scrubbed_path>(27,13-16): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
