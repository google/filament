#version 310 es


struct modf_result_f32 {
  float member_0;
  float whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_result_f32 res = modf_result_f32(0.25f, 1.0f);
  float v = res.member_0;
  float whole = res.whole;
}
