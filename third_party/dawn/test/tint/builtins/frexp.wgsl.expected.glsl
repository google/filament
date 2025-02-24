#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_result_f32 res = frexp_result_f32(0.61500000953674316406f, 1);
  int v = res.member_1;
  float v_1 = res.member_0;
}
