SKIP: INVALID

void set_matrix_scalar(inout float4x3 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[0];
      break;
    case 1:
      mat[1] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[1];
      break;
    case 2:
      mat[2] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[2];
      break;
    case 3:
      mat[3] = (row.xxx == int3(0, 1, 2)) ? val.xxx : mat[3];
      break;
  }
}

cbuffer cbuffer_x_9 : register(b0) {
  uint4 x_9[1];
};
static float4 x_GLF_color = float4(0.0f, 0.0f, 0.0f, 0.0f);

void main_1() {
  int idx = 0;
  float4x3 m43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_1 = 0;
  int GLF_live6rows = 0;
  int z = 0;
  int ll_2 = 0;
  int ctr = 0;
  float4x3 tempm43 = float4x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  int ll_3 = 0;
  int c = 0;
  int d = 0;
  float GLF_live6sums[9] = (float[9])0;
  idx = 0;
  m43 = float4x3(float3(1.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.0f, 1.0f), (0.0f).xxx);
  ll_1 = 0;
  GLF_live6rows = 2;
  while (true) {
    if ((ll_1 >= asint(x_9[0].x))) {
      x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
      break;
    }
    ll_1 = (ll_1 + 1);
    z = asint(x_9[0].x);
    ll_2 = 0;
    ctr = 0;
    while (true) {
      if ((ctr < 1)) {
      } else {
        break;
      }
      if ((ll_2 >= asint(x_9[0].x))) {
        break;
      }
      ll_2 = (ll_2 + 1);
      tempm43 = m43;
      ll_3 = 0;
      c = 0;
      while (true) {
        if ((1 < z)) {
        } else {
          break;
        }
        d = 0;
        int x_29 = c;
        int x_30 = c;
        int x_31 = c;
        int x_32 = d;
        int x_33 = d;
        int x_34 = d;
        set_matrix_scalar(tempm43, (((x_29 >= 0) & (x_30 < 4)) ? x_31 : 0), (((x_32 >= 0) & (x_33 < 3)) ? x_34 : 0), 1.0f);
        {
          c = (c + 1);
        }
      }
      int x_117 = (((idx >= 0) & (idx < 9)) ? idx : 0);
      GLF_live6sums[x_117] = (GLF_live6sums[x_117] + m43[ctr].y);
      {
        ctr = (ctr + 1);
      }
    }
    idx = (idx + 1);
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
hlsl.hlsl:102: error: Loop must have break.
Validation failed.



tint executable returned error: exit status 1
