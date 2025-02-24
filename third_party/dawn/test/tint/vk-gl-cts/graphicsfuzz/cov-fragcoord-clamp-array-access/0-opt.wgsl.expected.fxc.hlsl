SKIP: FAILED

int tint_ftoi(float v) {
  return ((v < 2147483520.0f) ? ((v < -2147483648.0f) ? -2147483648 : int(v)) : 2147483647);
}

int tint_clamp(int e, int low, int high) {
  return min(max(e, low), high);
}

cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
cbuffer cbuffer_x_10 : register(b1) {
  uint4 x_10[4];
};
static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  float4 data[2] = (float4[2])0;
  int b = 0;
  int y = 0;
  int i = 0;
  float x_42 = asfloat(x_7[0].x);
  float x_45 = asfloat(x_7[0].x);
  float4 tint_symbol_3[2] = {float4(x_42, x_42, x_42, x_42), float4(x_45, x_45, x_45, x_45)};
  data = tint_symbol_3;
  int x_49 = asint(x_10[1].x);
  b = x_49;
  float x_51 = gl_FragCoord.y;
  int x_54 = asint(x_10[1].x);
  float x_56 = gl_FragCoord.y;
  int x_60 = asint(x_10[1].x);
  y = tint_clamp(tint_ftoi(x_51), (x_54 | tint_ftoi(x_56)), x_60);
  int x_63 = asint(x_10[1].x);
  i = x_63;
  while (true) {
    bool x_82 = false;
    bool x_83_phi = false;
    int x_68 = i;
    int x_70 = asint(x_10[0].x);
    if ((x_68 < x_70)) {
    } else {
      break;
    }
    int x_73 = b;
    int x_75 = asint(x_10[0].x);
    bool x_76 = (x_73 > x_75);
    x_83_phi = x_76;
    if (x_76) {
      int x_79 = y;
      int x_81 = asint(x_10[1].x);
      x_82 = (x_79 > x_81);
      x_83_phi = x_82;
    }
    bool x_83 = x_83_phi;
    if (x_83) {
      break;
    }
    int x_86 = b;
    b = (x_86 + 1);
    {
      int x_88 = i;
      i = (x_88 + 1);
    }
  }
  int x_90 = b;
  int x_92 = asint(x_10[0].x);
  if ((x_90 == x_92)) {
    int x_97 = asint(x_10[2].x);
    int x_99 = asint(x_10[1].x);
    int x_101 = asint(x_10[3].x);
    int x_104 = asint(x_10[1].x);
    int x_107 = asint(x_10[2].x);
    int x_110 = asint(x_10[2].x);
    int x_113 = asint(x_10[1].x);
    data[tint_clamp(x_97, x_99, x_101)] = float4(float(x_104), float(x_107), float(x_110), float(x_113));
  }
  int x_118 = asint(x_10[1].x);
  float4 x_120 = data[x_118];
  x_GLF_color = float4(x_120.x, x_120.y, x_120.z, x_120.w);
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
  main_out tint_symbol_4 = {x_GLF_color};
  return tint_symbol_4;
}

tint_symbol_2 main(tint_symbol_1 tint_symbol) {
  main_out inner_result = main_inner(float4(tint_symbol.gl_FragCoord_param.xyz, (1.0f / tint_symbol.gl_FragCoord_param.w)));
  tint_symbol_2 wrapper_result = (tint_symbol_2)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}

tint executable returned error: exit status 0xc00000fd
