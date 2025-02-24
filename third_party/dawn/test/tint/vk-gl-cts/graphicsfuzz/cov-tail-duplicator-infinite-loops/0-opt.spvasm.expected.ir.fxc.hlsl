SKIP: INVALID

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static float4 x_GLF_color = (0.0f).xxxx;
cbuffer cbuffer_x_5 : register(b1) {
  uint4 x_5[2];
};
cbuffer cbuffer_x_7 : register(b2) {
  uint4 x_7[1];
};
cbuffer cbuffer_x_10 : register(b0) {
  uint4 x_10[2];
};
void main_1() {
  int i = int(0);
  x_GLF_color = float4((asfloat(x_5[0u].x)).xxxx);
  float v = asfloat(x_7[0u].x);
  if ((v > asfloat(x_5[0u].x))) {
    {
      while(true) {
        x_GLF_color = float4((asfloat(x_5[1u].x)).xxxx);
        {
          if (false) { break; }
        }
        continue;
      }
    }
  } else {
    {
      while(true) {
        {
          while(true) {
            if (true) {
            } else {
              break;
            }
            i = asint(x_10[1u].x);
            {
              while(true) {
                int v_1 = i;
                if ((v_1 < asint(x_10[0u].x))) {
                } else {
                  break;
                }
                float v_2 = asfloat(x_5[1u].x);
                float v_3 = asfloat(x_5[0u].x);
                float v_4 = asfloat(x_5[0u].x);
                x_GLF_color = float4(v_2, v_3, v_4, asfloat(x_5[1u].x));
                {
                  i = (i + int(1));
                }
                continue;
              }
            }
            break;
          }
        }
        {
          float x_82 = asfloat(x_7[0u].x);
          float x_84 = asfloat(x_5[0u].x);
          if (!((x_82 > x_84))) { break; }
        }
        continue;
      }
    }
  }
}

main_out main_inner() {
  main_1();
  main_out v_5 = {x_GLF_color};
  return v_5;
}

main_outputs main() {
  main_out v_6 = main_inner();
  main_outputs v_7 = {v_6.x_GLF_color_1};
  return v_7;
}

FXC validation failure:
<scrubbed_path>(26,13-16): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
