#version 310 es


struct modf_result_f32 {
  float member_0;
  float whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  float v = 1.25f;
  modf_result_f32 v_1 = modf_result_f32(0.0f, 0.0f);
  v_1.member_0 = modf(v, v_1.whole);
  modf_result_f32 res = v_1;
  float v_2 = res.member_0;
  float whole = res.whole;
}
