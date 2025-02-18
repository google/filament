#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool a = true;
  uint v_1 = uint(a);
  bool v = mix(bool((v_1 & uint(true))), true, false);
}
