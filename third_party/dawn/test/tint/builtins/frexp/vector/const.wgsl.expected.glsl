#version 310 es


struct frexp_result_vec2_f32 {
  vec2 member_0;
  ivec2 member_1;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  frexp_result_vec2_f32 res = frexp_result_vec2_f32(vec2(0.625f, 0.9375f), ivec2(1, 2));
  vec2 v = res.member_0;
  ivec2 v_1 = res.member_1;
}
