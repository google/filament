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
  float4 tint_symbol_3[2] = {float4((asfloat(x_7[0].x)).xxxx), float4((asfloat(x_7[0].x)).xxxx)};
  data = tint_symbol_3;
  b = asint(x_10[1].x);
  y = tint_clamp(tint_ftoi(gl_FragCoord.y), (asint(x_10[1].x) | tint_ftoi(gl_FragCoord.y)), asint(x_10[1].x));
  i = asint(x_10[1].x);
  while (true) {
    bool x_82 = false;
    bool x_83 = false;
    if ((i < asint(x_10[0].x))) {
    } else {
      break;
    }
    bool x_76 = (b > asint(x_10[0].x));
    x_83 = x_76;
    if (x_76) {
      x_82 = (y > asint(x_10[1].x));
      x_83 = x_82;
    }
    if (x_83) {
      break;
    }
    b = (b + 1);
    {
      i = (i + 1);
    }
  }
  if ((b == asint(x_10[0].x))) {
    int x_97 = asint(x_10[2].x);
    int x_99 = asint(x_10[1].x);
    int x_101 = asint(x_10[3].x);
    data[tint_clamp(x_97, x_99, x_101)] = float4(float(asint(x_10[1].x)), float(asint(x_10[2].x)), float(asint(x_10[2].x)), float(asint(x_10[1].x)));
  }
  float4 x_120 = data[asint(x_10[1].x)];
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
