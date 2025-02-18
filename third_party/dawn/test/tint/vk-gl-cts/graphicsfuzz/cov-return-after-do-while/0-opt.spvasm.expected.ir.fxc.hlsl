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
  float v = float(asint(x_5[0u].x));
  float v_1 = float(asint(x_5[1u].x));
  float v_2 = float(asint(x_5[1u].x));
  x_GLF_color = float4(v, v_1, v_2, float(asint(x_5[0u].x)));
  int v_3 = asint(x_5[1u].x);
  if ((v_3 > asint(x_5[0u].x))) {
    {
      while(true) {
        x_GLF_color = float4((float(asint(x_5[0u].x))).xxxx);
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
  main_out v_4 = {x_GLF_color};
  return v_4;
}

main_outputs main() {
  main_out v_5 = main_inner();
  main_outputs v_6 = {v_5.x_GLF_color_1};
  return v_6;
}

FXC validation failure:
<scrubbed_path>(22,13-16): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
