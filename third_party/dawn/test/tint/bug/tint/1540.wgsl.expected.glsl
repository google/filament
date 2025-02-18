#version 310 es


struct S {
  bool e;
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main() {
  bool b = false;
  bool v_1 = b;
  uint v_2 = uint(true);
  S v = S(bool((v_2 & uint(v_1))));
}
