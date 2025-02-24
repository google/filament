SKIP: INVALID

struct main_out {
  float4 x_GLF_color_1;
};

struct main_outputs {
  float4 main_out_x_GLF_color_1 : SV_Target0;
};


static float4 x_GLF_color = (0.0f).xxxx;
cbuffer cbuffer_x_6 : register(b0) {
  uint4 x_6[1];
};
void main_1() {
  int GLF_dead6index = int(0);
  int GLF_dead6currentNode = int(0);
  int donor_replacementGLF_dead6tree[1] = (int[1])0;
  x_GLF_color = float4(1.0f, 0.0f, 0.0f, 1.0f);
  GLF_dead6index = int(0);
  if ((asfloat(x_6[0u].y) < 0.0f)) {
    {
      while(true) {
        if (true) {
        } else {
          break;
        }
        int v = GLF_dead6index;
        GLF_dead6currentNode = donor_replacementGLF_dead6tree[v];
        GLF_dead6index = GLF_dead6currentNode;
        {
        }
        continue;
      }
    }
  }
}

main_out main_inner() {
  main_1();
  main_out v_1 = {x_GLF_color};
  return v_1;
}

main_outputs main() {
  main_out v_2 = main_inner();
  main_outputs v_3 = {v_2.x_GLF_color_1};
  return v_3;
}

FXC validation failure:
<scrubbed_path>(22,13-16): error X3696: infinite loop detected - loop never exits


tint executable returned error: exit status 1
