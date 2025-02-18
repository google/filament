#version 310 es


struct modf_result_f32 {
  float member_0;
  float whole;
};

modf_result_f32 v = modf_result_f32(0.0f, 1.0f);
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
