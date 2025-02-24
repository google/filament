#version 310 es


struct S {
  vec3 val[3];
};

void a() {
  ivec4 a_1 = ivec4(0);
  int b = a_1.x;
  ivec4 c = a_1.zzyy;
  S d = S(vec3[3](vec3(0.0f), vec3(0.0f), vec3(0.0f)));
  vec3 e = d.val[2u].yzx;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
