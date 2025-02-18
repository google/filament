SKIP: INVALID

static float4 gl_FragCoord = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

float fx_() {
  if ((gl_FragCoord.y >= 0.0f)) {
    float x_55 = asfloat(x_7[0].y);
    return x_55;
  }
  while (true) {
    if (true) {
    } else {
      break;
    }
    x_GLF_color = (1.0f).xxxx;
  }
  return 0.0f;
}

void main_1() {
  float x2 = 0.0f;
  float B = 0.0f;
  float k0 = 0.0f;
  x2 = 1.0f;
  B = 1.0f;
  float x_34 = fx_();
  x_GLF_color = float4(x_34, 0.0f, 0.0f, 1.0f);
  while (true) {
    if ((x2 > 2.0f)) {
    } else {
      break;
    }
    float x_43 = fx_();
    float x_44 = fx_();
    k0 = (x_43 - x_44);
    B = k0;
    x2 = B;
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
<scrubbed_path>(12,10-13): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
