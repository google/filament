SKIP: FAILED

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};

struct main_inputs {
  float4 gl_FragCoord_param : SV_Position;
};


cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
cbuffer cbuffer_x_10 : register(b1) {
  uint4 x_10[4];
};
static float4 gl_FragCoord = (0.0f).xxxx;
static float4 x_GLF_color = (0.0f).xxxx;
int tint_f32_to_i32(float value) {
  return (((value <= 2147483520.0f)) ? ((((value >= -2147483648.0f)) ? (int(value)) : (int(-2147483648)))) : (int(2147483647)));
}

void main_1() {
  float4 data[2] = (float4[2])0;
  int b = int(0);
  int y = int(0);
  int i = int(0);
  float4 v = float4((asfloat(x_7[0u].x)).xxxx);
  float4 v_1[2] = {v, float4((asfloat(x_7[0u].x)).xxxx)};
  data = v_1;
  b = asint(x_10[1u].x);
  int v_2 = tint_f32_to_i32(gl_FragCoord.y);
  int v_3 = asint(x_10[1u].x);
  int v_4 = (v_3 | tint_f32_to_i32(gl_FragCoord.y));
  int v_5 = asint(x_10[1u].x);
  y = min(max(v_2, v_4), v_5);
  i = asint(x_10[1u].x);
  {
    while(true) {
      bool x_82 = false;
      bool x_83 = false;
      int v_6 = i;
      if ((v_6 < asint(x_10[0u].x))) {
      } else {
        break;
      }
      int v_7 = b;
      bool x_76 = (v_7 > asint(x_10[0u].x));
      x_83 = x_76;
      if (x_76) {
        int v_8 = y;
        x_82 = (v_8 > asint(x_10[1u].x));
        x_83 = x_82;
      }
      if (x_83) {
        break;
      }
      b = (b + int(1));
      {
        i = (i + int(1));
      }
      continue;
    }
  }
  int v_9 = b;
  if ((v_9 == asint(x_10[0u].x))) {
    int x_97 = asint(x_10[2u].x);
    int x_99 = asint(x_10[1u].x);
    int x_101 = asint(x_10[3u].x);
    int v_10 = min(max(x_97, x_99), x_101);
    float v_11 = float(asint(x_10[1u].x));
    float v_12 = float(asint(x_10[2u].x));
    float v_13 = float(asint(x_10[2u].x));
    data[v_10] = float4(v_11, v_12, v_13, float(asint(x_10[1u].x)));
  }
  int v_14 = asint(x_10[1u].x);
  float4 x_120 = data[v_14];
  x_GLF_color = float4(x_120[0u], x_120[1u], x_120[2u], x_120[3u]);
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v_15 = {x_GLF_color};
  return v_15;
}

main_outputs main(main_inputs inputs) {
  main_out v_16 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_17 = {v_16.x_GLF_color_1};
  return v_17;
}


tint executable returned error: exit status 0xc00000fd
