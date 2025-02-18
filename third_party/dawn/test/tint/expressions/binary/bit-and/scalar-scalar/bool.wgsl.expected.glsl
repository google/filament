#version 310 es

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool a = true;
  bool b = false;
  uint v = uint(a);
  bool r = bool((v & uint(b)));
}
