#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_result_f32 res = frexp_result_f32(0.625f, 1);
  float v = res.member_0;
  int v_1 = res.member_1;
}
