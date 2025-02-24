#version 310 es


struct modf_result_vec2_f32 {
  vec2 member_0;
  vec2 whole;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec2 v = vec2(1.25f, 3.75f);
  modf_result_vec2_f32 v_1 = modf_result_vec2_f32(vec2(0.0f), vec2(0.0f));
  v_1.member_0 = modf(v, v_1.whole);
  modf_result_vec2_f32 res = v_1;
  vec2 v_2 = res.member_0;
  vec2 whole = res.whole;
}
