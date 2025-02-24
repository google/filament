SKIP: INVALID

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static float4 x_GLF_color = (0.0f).xxxx;
cbuffer cbuffer_x_5 : register(b0) {
  uint4 x_5[1];
};
void main_1() {
  int m = int(0);
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  float v = asfloat(x_5[0u].x);
  if ((v > asfloat(x_5[0u].y))) {
    {
      while(true) {
        {
          if (true) { break; }
        }
        continue;
      }
    }
    m = int(1);
    {
      while(true) {
        if (true) {
        } else {
          break;
        }
        x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
        {
        }
        continue;
      }
    }
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

FXC validation failure:
<scrubbed_path>(20,7-17): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(29,13-16): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
