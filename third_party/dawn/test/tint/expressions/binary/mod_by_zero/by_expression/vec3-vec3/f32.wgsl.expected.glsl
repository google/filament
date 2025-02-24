#version 310 es

vec3 tint_float_modulo(vec3 x, vec3 y) {
  return (x - (y * trunc((x / y))));
}
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  vec3 a = vec3(1.0f, 2.0f, 3.0f);
  vec3 b = vec3(0.0f, 5.0f, 0.0f);
  vec3 r = tint_float_modulo(a, (b + b));
}
