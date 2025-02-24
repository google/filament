SKIP: FAILED

int tint_ftoi(float v) {
  return ((v < 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  while (true) {
    bool x_45 = false;
    int x_48 = 0;
    int x_50 = 0;
    int x_52 = 0;
    int x_49 = 0;
    bool x_46 = false;
    int x_111 = 0;
    bool x_112 = false;
    int x_115 = 0;
    int x_118 = 0;
    int x_120 = 0;
    int x_116 = 0;
    int x_161 = 0;
    float x_40 = asfloat(x_6[0].x);
    bool x_41 = (x_40 < -1.0f);
    x_45 = false;
    x_48 = 0;
    x_50 = 0;
    x_52 = 0;
    while (true) {
      int x_62 = 0;
      int x_65 = 0;
      int x_67 = 0;
      int x_66 = 0;
      int x_63 = 0;
      int x_51 = 0;
      int x_53 = 0;
      float x_55 = gl_FragCoord.y;
      x_111 = x_48;
      x_112 = x_45;
      if ((x_52 < ((x_55 > -1.0f) ? 10 : 100))) {
      } else {
        break;
      }
      x_62 = x_48;
      x_65 = x_50;
      x_67 = 0;
      while (true) {
        int x_97 = 0;
        int x_68 = 0;
        x_51 = x_65;
        x_49 = x_62;
        x_46 = x_45;
        if ((x_67 < 2)) {
        } else {
          break;
        }
        while (true) {
          bool x_78 = false;
          int x_86 = 0;
          bool x_98 = false;
          x_78 = (gl_FragCoord.x < -1.0f);
          if (!((x_40 < 0.0f))) {
            if (x_78) {
              x_66 = 0;
              break;
            }
            x_86 = 1;
            while (true) {
              int x_87 = 0;
              x_97 = x_65;
              x_98 = false;
              if ((x_86 < 3)) {
              } else {
                break;
              }
              if (x_78) {
                {
                  x_87 = (x_86 + 1);
                  x_86 = x_87;
                }
                continue;
              }
              if ((x_86 > 0)) {
                x_97 = 1;
                x_98 = true;
                break;
              }
              {
                x_87 = (x_86 + 1);
                x_86 = x_87;
              }
            }
            x_66 = x_97;
            if (x_98) {
              break;
            }
          }
          x_66 = 0;
          break;
        }
        x_63 = (x_62 + x_66);
        if (x_41) {
          while (true) {
            if (x_41) {
            } else {
              break;
            }
            {
              x_GLF_color = float4((float(x_52)).xxxx);
            }
          }
          x_51 = x_66;
          x_49 = x_63;
          x_46 = true;
          break;
        }
        {
          x_68 = (x_67 + 1);
          x_62 = x_63;
          x_65 = x_66;
          x_67 = x_68;
        }
      }
      x_111 = x_49;
      x_112 = x_46;
      if (x_46) {
        break;
      }
      if (!(x_41)) {
        x_111 = x_49;
        x_112 = x_46;
        break;
      }
      {
        x_53 = (x_52 + 1);
        x_45 = x_46;
        x_48 = x_49;
        x_50 = x_51;
        x_52 = x_53;
      }
    }
    if (x_112) {
      break;
    }
    x_115 = x_111;
    x_118 = 0;
    x_120 = 0;
    while (true) {
      int x_154 = 0;
      int x_119 = 0;
      int x_121 = 0;
      float x_123 = asfloat(x_6[0].y);
      x_161 = x_115;
      if ((x_120 < tint_ftoi((x_123 + 1.0f)))) {
      } else {
        break;
      }
      while (true) {
        bool x_135 = false;
        int x_143 = 0;
        bool x_155 = false;
        x_135 = (gl_FragCoord.x < -1.0f);
        if (!((x_40 < 0.0f))) {
          if (x_135) {
            x_119 = 0;
            break;
          }
          x_143 = 1;
          while (true) {
            int x_144 = 0;
            x_154 = x_118;
            x_155 = false;
            if ((x_143 < 3)) {
            } else {
              break;
            }
            if (x_135) {
              {
                x_144 = (x_143 + 1);
                x_143 = x_144;
              }
              continue;
            }
            if ((x_143 > 0)) {
              x_154 = 1;
              x_155 = true;
              break;
            }
            {
              x_144 = (x_143 + 1);
              x_143 = x_144;
            }
          }
          x_119 = x_154;
          if (x_155) {
            break;
          }
        }
        x_119 = 0;
        break;
      }
      x_116 = (x_115 + x_119);
      if ((!(x_41) ? false : x_41)) {
        x_161 = x_116;
        break;
      }
      {
        x_121 = (x_120 + 1);
        x_115 = x_116;
        x_118 = x_119;
        x_120 = x_121;
      }
    }
    if ((x_161 == 4)) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    } else {
      x_GLF_color = (0.0f).xxxx;
    }
    break;
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol_1 {
  float4 gl_FragCoord_param : SV_Position;
};
struct tint_symbol_2 {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out tint_symbol_3 = {x_GLF_color};
  return tint_symbol_3;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  main_out inner_result = main_inner(float4(tint_symbol.gl_FragCoord_param.xyz, (1.0f / tint_symbol.gl_FragCoord_param.w)));
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(12,3-14): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(60,9-20): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(161,7-18): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(60,9-20): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(161,7-18): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(106,11-22): error X4029: infinite loop detected - loop never exits


tint executable returned error: exit status 1
