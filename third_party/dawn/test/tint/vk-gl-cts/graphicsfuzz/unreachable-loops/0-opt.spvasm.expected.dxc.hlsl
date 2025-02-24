SKIP: INVALID

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
cbuffer cbuffer_x_5 : register(b0) {
  uint4 x_5[1];
};

void main_1() {
  int m = 0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  if ((asfloat(x_5[0].x) > asfloat(x_5[0].y))) {
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
DXC validation failure:
error: validation errors
hlsl.hlsl:40: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
