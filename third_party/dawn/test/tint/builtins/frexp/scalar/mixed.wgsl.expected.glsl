#version 310 es


struct frexp_result_f32 {
  float member_0;
  int member_1;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float runtime_in = 1.25f;
  frexp_result_f32 res = frexp_result_f32(0.625f, 1);
  frexp_result_f32 v = frexp_result_f32(0.0f, 0);
  v.member_0 = frexp(runtime_in, v.member_1);
  res = v;
  res = frexp_result_f32(0.625f, 1);
  float v_1 = res.member_0;
  int v_2 = res.member_1;
}
