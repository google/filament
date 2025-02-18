#version 310 es


struct modf_result_f32 {
  float member_0;
  float whole;
};

void foo() {
  modf_result_f32 s1 = modf_result_f32(0.0f, 0.0f);
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
