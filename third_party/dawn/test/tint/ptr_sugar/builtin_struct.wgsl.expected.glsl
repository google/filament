#version 310 es


struct modf_result_f32 {
  float member_0;
  float whole;
};

struct frexp_result_f32 {
  float member_0;
  int member_1;
};

void deref_modf() {
  modf_result_f32 a = modf_result_f32(0.5f, 1.0f);
  float v = a.member_0;
  float whole = a.whole;
}
void no_deref_modf() {
  modf_result_f32 a = modf_result_f32(0.5f, 1.0f);
  float v_1 = a.member_0;
  float whole = a.whole;
}
void deref_frexp() {
  frexp_result_f32 a = frexp_result_f32(0.75f, 1);
  float v_2 = a.member_0;
  int v_3 = a.member_1;
}
void no_deref_frexp() {
  frexp_result_f32 a = frexp_result_f32(0.75f, 1);
  float v_4 = a.member_0;
  int v_5 = a.member_1;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  deref_modf();
  no_deref_modf();
  deref_frexp();
  no_deref_frexp();
}
