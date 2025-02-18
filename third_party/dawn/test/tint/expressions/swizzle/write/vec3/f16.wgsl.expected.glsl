#version 310 es
#extension GL_AMD_gpu_shader_half_float: require


struct S {
  f16vec3 v;
};

S P = S(f16vec3(0.0hf));
void f() {
  P.v = f16vec3(1.0hf, 2.0hf, 3.0hf);
  P.v.x = 1.0hf;
  P.v.y = 2.0hf;
  P.v.z = 3.0hf;
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
}
