SKIP: INVALID

static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);
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
  int i = 0;
  x_GLF_color = float4((asfloat(x_5[0].x)).xxxx);
  if ((asfloat(x_7[0].x) > asfloat(x_5[0].x))) {
    while (true) {
      x_GLF_color = float4((asfloat(x_5[1].x)).xxxx);
      {
        if (false) { break; }
      }
    }
  } else {
    while (true) {
      while (true) {
        if (true) {
        } else {
          break;
        }
        i = asint(x_10[1].x);
        while (true) {
          if ((i < asint(x_10[0].x))) {
          } else {
            break;
          }
          x_GLF_color = float4(asfloat(x_5[1].x), asfloat(x_5[0].x), asfloat(x_5[0].x), asfloat(x_5[1].x));
          {
            i = (i + 1);
          }
        }
        break;
      }
      {
        float x_82 = asfloat(x_7[0].x);
        float x_84 = asfloat(x_5[0].x);
        if (!((x_82 > x_84))) { break; }
      }
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
hlsl.hlsl:65: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
