#version 310 es


struct modf_result_vec2_f32 {
  vec2 member_0;
  vec2 whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  modf_result_vec2_f32 res = modf_result_vec2_f32(vec2(0.25f, 0.75f), vec2(1.0f, 3.0f));
  vec2 v = res.member_0;
  vec2 whole = res.whole;
}
