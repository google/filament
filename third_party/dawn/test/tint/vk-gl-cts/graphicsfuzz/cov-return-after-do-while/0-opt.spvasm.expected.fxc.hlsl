SKIP: INVALID

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0) {
  uint4 x_5[2];
};

void main_1() {
  x_GLF_color = float4(float(asint(x_5[0].x)), float(asint(x_5[1].x)), float(asint(x_5[1].x)), float(asint(x_5[0].x)));
  if ((asint(x_5[1].x) > asint(x_5[0].x))) {
    while (true) {
      x_GLF_color = float4((float(asint(x_5[0].x))).xxxx);
      {
        int x_50 = asint(x_5[1].x);
        int x_52 = asint(x_5[0].x);
        if (!((x_50 > x_52))) { break; }
      }
    }
    return;
  }
  return;
}

struct main_out {
  float4 x_GLF_color_1;
};
struct tint_symbol {
  float4 x_GLF_color_1 : SV_Target0;
};

main_out main_inner() {
  main_1();
  main_out tint_symbol_1 = {x_GLF_color};
  return tint_symbol_1;
}

tint_symbol main() {
  main_out inner_result = main_inner();
  tint_symbol wrapper_result = (tint_symbol)0;
  wrapper_result.x_GLF_color_1 = inner_result.x_GLF_color_1;
  return wrapper_result;
}
FXC validation failure:
<scrubbed_path>(9,12-15): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
