SKIP: INVALID

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0) {
  uint4 x_5[1];
};

void main_1() {
  int m = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  float x_30 = asfloat(x_5[0].x);
  float x_32 = asfloat(x_5[0].y);
  if ((x_30 > x_32)) {
    while (true) {
      {
        if (true) { break; }
      }
    }
    m = 1;
    while (true) {
      if (true) {
      } else {
        break;
      }
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    }
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
<scrubbed_path>(12,5-16): warning X3557: loop only executes for 0 iteration(s), forcing loop to unroll
<scrubbed_path>(18,12-15): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
