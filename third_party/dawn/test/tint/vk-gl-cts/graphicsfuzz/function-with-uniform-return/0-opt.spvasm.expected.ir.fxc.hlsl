SKIP: INVALID

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};

struct main_inputs {
  float4 gl_FragCoord_param : SV_Position;
};


static float4 gl_FragCoord = (0.0f).xxxx;
cbuffer cbuffer_x_7 : register(b0) {
  uint4 x_7[1];
};
static float4 x_GLF_color = (0.0f).xxxx;
float fx_() {
  if ((gl_FragCoord.y >= 0.0f)) {
    float x_55 = asfloat(x_7[0u].y);
    return x_55;
  }
  {
    while(true) {
      if (true) {
      } else {
        break;
      }
      x_GLF_color = (1.0f).xxxx;
      {
      }
      continue;
    }
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
  {
    while(true) {
      if ((x2 > 2.0f)) {
      } else {
        break;
      }
      float x_43 = fx_();
      float x_44 = fx_();
      k0 = (x_43 - x_44);
      B = k0;
      x2 = B;
      {
      }
      continue;
    }
  }
}

main_out main_inner(float4 gl_FragCoord_param) {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  main_out v = {x_GLF_color};
  return v;
}

main_outputs main(main_inputs inputs) {
  main_out v_1 = main_inner(float4(inputs.gl_FragCoord_param.xyz, (1.0f / inputs.gl_FragCoord_param[3u])));
  main_outputs v_2 = {v_1.x_GLF_color_1};
  return v_2;
}

FXC validation failure:
<scrubbed_path>(25,11-14): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
